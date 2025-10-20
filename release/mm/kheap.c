#ifndef _KHEAP_C
#define _KHEAP_C

#include <mm/kheap.h>
#include <mm/vmm.h>
#include <utils.h>
#include <string.h>
#include <log.h>

// define internal kernel heap state
static kheap_state_t _kernel_kheap_state;

// define the kernel heap
heap_t kernel_heap;


void kheap_init(heap_t *heap, void *start, size_t size, size_t max_size, bool is_supervisor, bool is_readonly)
{
    LOG_DEBUG("------------------------------\n");
    LOG_DEBUG("KHEAP INIT @0x%08x\n", (uintptr_t)start);

    if (!heap)
    {
        LOG_ERROR("kheap_init: heap is NULL\n");
        return;
    }

    // some other sanity checks (somebody please check my sanity)
    if (!start || size == 0)
    {
        LOG_ERROR("kheap_init: invalid start/size\n");
        return;
    }

    // align start and end pages
    uintptr_t start_addr = (uintptr_t)start;
    uintptr_t aligned_start = (uintptr_t) ALIGN(start_addr, VMM_PAGE_SIZE);
    uintptr_t aligned_end   = (uintptr_t) ALIGN(start_addr + size, VMM_PAGE_SIZE);

    // sanity check
    if (aligned_end <= aligned_start)
        return;


    // get usable space
    size_t usable = aligned_end - aligned_start;

    // choose the largest power-of-two (order)
    unsigned highest = _compute_highest_exponent(usable);
    size_t managed_size = ((size_t)1) << highest;


    // sanity checks for order
    if (highest < BUDDY_MIN_ORDER)
    {
        LOG_ERROR("kheap_init: usable region too small for min order %u\n", BUDDY_MIN_ORDER);
        return;    // ruswai
    }

    if (highest >= BUDDY_MAX_ORDER)
    {
        LOG_ERROR("kheap_init: computed order %u >= BUDDY_MAX_ORDER (%u)\n", highest, BUDDY_MAX_ORDER);
        return;
    }

    // init heap descriptor
    heap->state = &_kernel_kheap_state;
    heap->start = aligned_start;
    heap->end   = aligned_start + managed_size;
    heap->max_size = (uint32_t) ((max_size == 0 || (size_t)max_size > managed_size) ? managed_size : max_size);
    heap->is_supervisor = (uint8_t) is_supervisor;
    heap->is_readonly = (uint8_t) is_readonly;


    // get buddy state
    kheap_state_t *st = (kheap_state_t*) heap->state;

    // clear out memory region
    memset(st, 0, sizeof(*st));

    // init state
    st->base = (uintptr_t) heap->start;
    st->size = managed_size;
    st->min_order = BUDDY_MIN_ORDER;
    st->max_order = highest;


    // time to map heap onto address space
    pagedir_t *map_pdir = vmm_get_kerneldir();

    if (!map_pdir)
        map_pdir = vmm_get_current_pagedir();

    uint32_t map_flags = PTE_PRESENT | PTE_WRITABLE;

    // check for user
    if (!is_supervisor)
        map_flags |= PTE_USER;


    // try allocating the heap into pagedir
    bool alloc = vmm_alloc_region(map_pdir, (void*)st->base, st->size, map_flags);

    if (!alloc)
        return;    // zaleel


    // vibes
    /* seed the free-lists with a single root free block covering the managed region */
    free_block_hdr *root = (free_block_hdr*) (st->base);
    root->prev = NULL;
    root->next = NULL;

    // vibes??
    uint32_t idx = st->max_order - st->min_order;
    st->free_lists[idx] = root;

}

void* kmalloc(heap_t *heap, size_t size)
{
    // check heap ptr
    if (!heap)
    {
        LOG_ERROR("kmalloc: heap is NULL\n");
        return NULL;
    }

    // check size
    if (size == 0)
    {
        LOG_DEBUG("kmalloc: zero-sized allocation requested -> returning NULL\n");
        return NULL;
    }

    // check internal state
    if (!heap->state)
    {
        LOG_ERROR("kmalloc: heap state is NULL\n");
        return NULL;
    }


    // get pointer to internal state
    kheap_state_t* st = (kheap_state_t*) heap->state;

    // get size of header
    size_t header_size = ALLOC_BLOCK_HDR_SIZE;

    // compute total number of bytes required
    size_t total_bytes_required = size + header_size;

    // check if it exceeds max size of the heap
    if (total_bytes_required > (size_t) heap->max_size)
    {
        LOG_DEBUG("kmalloc: total_bytes_required=%zu exceeds heap max=%u\n", total_bytes_required, heap->max_size);
        return NULL;
    }

    // get smallest block order
    uint32_t target_block_order = st->min_order;
    size_t current_block_size = ((size_t)1) << target_block_order;

    while (current_block_size < total_bytes_required)
    {
        // increment target order
        target_block_order++;

        if (target_block_order > st->max_order)
        {
            // too big to fit (no order large enough)
            LOG_DEBUG("kmalloc: no block order large enough for total_bytes_required=%zu\n", total_bytes_required);
            return NULL;
        }

        // double block size
        current_block_size <<= 1; 
    }


    // search for a free block with the target order
    bool found = false;
    uint32_t found_order;

    for (uint32_t order = target_block_order; order <= st->max_order; ++order)
    {
        uint32_t list_index = order - st->min_order;

        // vibes
        if (list_index >= (uint32_t)(sizeof(st->free_lists) / sizeof(st->free_lists[0])))
            break; /* safety */


        if (st->free_lists[list_index] != NULL)
        {
            found = true;
            found_order = order;

            break;
        }
    }

    // check invalid block order
    if (!found)
    {
        LOG_DEBUG("kmalloc: out of memory (no free block found)\n");
        return NULL;
    }

    // using this block order, take the first free node
    uint32_t src_block_order = found_order;
    uint32_t src_list_index = src_block_order - st->min_order;


    free_block_hdr* alloc_node = st->free_lists[src_list_index];
    if (!alloc_node)
    {
        LOG_DEBUG("kmalloc: unexpected empty free-list at order=%u\n", src_block_order);
        return NULL;
    }

    // update links from alloc node (remove head of free-list)
    // set the free-list head to alloc_node->next
    st->free_lists[src_list_index] = alloc_node->next;

    // if new head exists, clear its prev
    if (st->free_lists[src_list_index])
    {
        st->free_lists[src_list_index]->prev = NULL;
    }

    // clear the removed node's pointers
    alloc_node->prev = alloc_node->next = NULL;

    uintptr_t allocated_block_base = (uintptr_t) alloc_node;


    // split until target order is reached
    while (src_block_order > target_block_order)
    {
        src_block_order--;

        size_t split_block_size = ((size_t)1) << src_block_order;

        // choosing RHS buddy (+offset instead of -offset)
        uintptr_t buddy_block_base = allocated_block_base + split_block_size;

        /* prepare buddy node and insert into appropriate free list */
        free_block_hdr* buddy_node = (free_block_hdr*) buddy_block_base;

        // new buddy will become head of free list
        buddy_node->prev = NULL;

        uint32_t buddy_list_index = src_block_order - st->min_order;

        // link buddy_node at the front of the free-list
        buddy_node->next = st->free_lists[buddy_list_index];

        // if there was an old head, update prev to point back to new head
        if (buddy_node->next)
        {
            buddy_node->next->prev = buddy_node;
        }

        // install buddy_node as new head
        st->free_lists[buddy_list_index] = buddy_node;

    }

    // mark final block as alloc
    alloc_block_hdr* alloc_header = (alloc_block_hdr*) allocated_block_base;
    alloc_header->size = size; /* user-visible size */
    alloc_header->magic = MAGIC_NO;

    // make sure to skip the header
    void* user_pointer = (void*)(allocated_block_base + header_size);

    // LOG_DEBUG("kmalloc: allocated user_pointer=0x%08x size=%zu order=%u\n", (uint32_t)(uintptr_t)user_pointer, size, target_block_order);

    return user_pointer;
}

void kfree(heap_t *heap, void* ptr)
{
    // LOG_DEBUG("kfree: ptr=0x%08x\n", (uint32_t)(uintptr_t)ptr);

    // validate parameters
    if (!heap || !ptr)
    {
        LOG_DEBUG("kfree: null heap or ptr -> ignoring\n");
        return;
    }

    if (!heap->state)
    {
        LOG_ERROR("kfree: heap state is NULL\n");
        return;
    }

    kheap_state_t *state = (kheap_state_t*) heap->state;

    // compute header start
    size_t header_size = ALLOC_BLOCK_HDR_SIZE;
    uintptr_t alloc_header_addr = (uintptr_t)ptr - header_size;

    uintptr_t heap_base_addr = state->base;
    uintptr_t heap_end_addr = state->base + state->size;

    if (alloc_header_addr < heap_base_addr || (alloc_header_addr + sizeof(alloc_block_hdr)) > heap_end_addr)
    {
        LOG_DEBUG("kfree: pointer 0x%08x not within heap range -> ignoring\n", (uint32_t)(uintptr_t)ptr);
        return;
    }

    // access header
    alloc_block_hdr *alloc_header = (alloc_block_hdr*) alloc_header_addr;

    // validate magic 
    if (alloc_header->magic != MAGIC_NO)
    {
        LOG_DEBUG("kfree: invalid or double-free detected (magic=0x%08x) -> ignoring\n", alloc_header->magic);
        return;
    }

    // compute block order from this
    size_t user_payload_size = alloc_header->size;
    size_t total_bytes_required = user_payload_size + header_size;

    unsigned block_order = state->min_order;
    size_t block_size_bytes = ((size_t)1) << block_order;

    while (block_size_bytes < total_bytes_required)
    {
        block_order++;

        if (block_order > state->max_order)
        {
            LOG_DEBUG("kfree: allocation size inconsistent with heap orders -> ignoring\n");
            return;
        }

        block_size_bytes <<= 1;
    }

    // ensure header alignment
    uintptr_t block_base_addr = alloc_header_addr;
    uintptr_t block_offset = block_base_addr - heap_base_addr;

    // // check 
    // if ((block_offset % block_size_bytes) != 0)
    // {
    //     LOG_DEBUG("kfree: block base 0x%08x not aligned to block size %zu -> ignoring\n", (uint32_t)block_base_addr, block_size_bytes);
    //     return;
    // }

    // clear magic
    alloc_header->magic = 0;

    
    // attempt to merge all possible buddies
    unsigned current_order = block_order;
    uintptr_t current_block_base_addr = block_base_addr;

    while (current_order < state->max_order)
    {
        size_t current_block_size_bytes = ((uint32_t)1) << current_order;

        // compute buddy offset (block offset ^ block_size)
        uintptr_t current_block_offset = current_block_base_addr - heap_base_addr;
        uintptr_t buddy_offset = current_block_offset ^ current_block_size_bytes;
        uintptr_t buddy_base_addr = heap_base_addr + buddy_offset;

        // check bounds of da buddy
        if (buddy_base_addr < heap_base_addr || (buddy_base_addr + current_block_size_bytes) > heap_end_addr)
        {
            break;
        }

        // search for buddy
        uint32_t list_idx = current_order - state->min_order;
        free_block_hdr *iter = state->free_lists[list_idx];
        free_block_hdr *found_buddy = NULL;

        // iterate in th list
        while (iter)
        {
            if ((uintptr_t)iter == buddy_base_addr)
            {
                // I FOUND YOU BUDDY!
                found_buddy = iter;
                break;
            }
            iter = iter->next;
        }

        // if buddy not free, we cannot merge
        if (!found_buddy)
        {
            break;
        }

        // remove found_buddy from its free-list
        // if buddy has a previous node, link prev->next to buddy->next
        if (found_buddy->prev)
        {
            found_buddy->prev->next = found_buddy->next;
        }
        else
        {
            // buddy was head of the list, update the list head
            state->free_lists[list_idx] = found_buddy->next;
        }

        // if buddy had a next node, link next->prev to buddy->prev
        if (found_buddy->next)
        {
            found_buddy->next->prev = found_buddy->prev;
        }

        // fully detach found buddy
        found_buddy->prev = NULL;
        found_buddy->next = NULL;

        // determine new merged block base (lower address)
        if (buddy_base_addr < current_block_base_addr)
        {
            current_block_base_addr = buddy_base_addr;
        }

        // check next order
        current_order++;
    }

    // insert the free block into free list
    uint32_t insert_idx = current_order - state->min_order;
    free_block_hdr *node = (free_block_hdr*) current_block_base_addr;

    // insert at head
    node->prev = NULL;
    node->next = state->free_lists[insert_idx];

    // if there was an existing head, set its prev pointer to the new node
    if (node->next)
        node->next->prev = node;

    // update the free-list head to point to the newly inserted node
    state->free_lists[insert_idx] = node;

    // LOG_DEBUG("kfree: inserted base=0x%08x order=%u size=%zu\n", (uint32_t)current_block_base_addr, current_order, ((size_t)1) << current_order);

    return;
}

void* krealloc(heap_t *heap, void *ptr, size_t new_size)
{
    // check heap
    if (!heap)
        return NULL;

    // check if ptr is NULL -> call kmalloc()
    if (!ptr && new_size != 0)
        return kmalloc(heap, new_size);

    // new_size is zero -> call kfree()
    if (new_size == 0)
    {
        kfree(heap, ptr);
        return NULL;
    }


    // need to check existing size
    if (!heap->state)
    {
        return NULL;
    }

    kheap_state_t* state = (kheap_state_t*) heap->state;

    // get address of header
    size_t header_size = ALLOC_BLOCK_HDR_SIZE;
    uintptr_t alloc_header_addr = (uintptr_t) (ptr - header_size);

    // get heap base and heap end addrs for bounds checking 
    uintptr_t heap_base_addr = state->base;
    uintptr_t heap_end_addr  = state->base + state->size;

    if (alloc_header_addr < heap_base_addr || (alloc_header_addr + sizeof(alloc_block_hdr)) > heap_end_addr)
    {
        // LOG_ERROR("krealloc: pointer 0x%08x not within heap\n", (uintptr_t)ptr);
        return NULL;
    }

    // get header state
    alloc_block_hdr* alloc_header = (alloc_block_hdr*) alloc_header_addr;

    // validate magic
    if (alloc_header->magic != MAGIC_NO)
    {
        LOG_ERROR("krealloc: invalid magic number!\n");
        return NULL;
    }

    
    // get the old size from the header
    size_t old_size = alloc_header->size;

    // compute the current block's order and size (block includes the header)
    size_t old_total_bytes = old_size + header_size;
    unsigned old_block_order = state->min_order;

    size_t old_block_size_bytes = ((size_t)1) << old_block_order;

    while (old_block_size_bytes < old_total_bytes)
    {
        old_block_order++;

        if (old_block_order > state->max_order)
        {
            LOG_ERROR("krealloc: computed block order out of range\n");
            return NULL;
        }

        old_block_size_bytes <<= 1;
    }

    // get original capacity
    size_t old_capacity = old_block_size_bytes - header_size;

    // new_size fits into the OG block capacity, no need to rizz the heap
    if (new_size <= old_capacity)
    {
        alloc_header->size = new_size;
        return ptr;
    }
    
    // need to rizz -> kmalloc()
    void* new_ptr = kmalloc(heap, new_size);

    if (!new_ptr)
    {
        LOG_ERROR("krealloc: alloc failed!\n");
        return NULL;
    }

    // copy the lesser of the old and new sizes
    size_t bytes_to_copy = (old_size < new_size) ? old_size : new_size;
    memcpy(new_ptr, ptr, bytes_to_copy);


    // free the old allocation
    kfree(heap, ptr);


    return new_ptr;

}

// helpers
heap_t* get_kernel_heap(void)
{
    return &kernel_heap;
}

void free(void* ptr)
{
    kfree(&kernel_heap, ptr);
}

uint32_t _compute_highest_exponent(size_t n)
{
    uint32_t p = 0;

    while (n >>= 1) 
        p++;

    return p;
}

#endif
