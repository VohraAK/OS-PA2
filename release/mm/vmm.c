#ifndef _VMM_C
#define _VMM_C

#include <mm/vmm.h>

// global state variables
static pagedir_t* _vmm_current_pagedir = NULL;
static pagedir_t* _vmm_kernel_pagedir = NULL;

// NOTE: page directories and tables MUST BE accessed through physmap

// Helper function to validate physical frame address
static inline bool _vmm_is_valid_frame(void* frame_phys)
{
    if (!frame_phys)
        return false;
    
    // Check page alignment
    if ((uintptr_t)frame_phys % VMM_PAGE_SIZE != 0)
        return false;
    
    // Check bounds: frame must be within valid physical memory range
    uint32_t max_phys_addr = kmm_get_total_frames() * VMM_PAGE_SIZE;
    if ((uintptr_t)frame_phys >= max_phys_addr)
        return false;
    
    return true;
}

void vmm_init(void)
{
    LOG_DEBUG("------------------------------\n");
    LOG_DEBUG("VMM INIT\n");

    // register page fault interrupt handler
    register_interrupt_handler(PAGE_FAULT_INTERRUPT, _vmm_page_fault_handler);

    // create new address space for the kernel
    pagedir_t* kernel_addr_space = vmm_create_address_space();

    // sanity check
    if (!kernel_addr_space)
        return;

    // set kernel_pagedir
    LOG_DEBUG("Set kernel page directory: 0x%08x\n", (uint32_t)kernel_addr_space);
    _vmm_kernel_pagedir = kernel_addr_space; 

    // set up identity mapping for first 1MB of physical memory (map each 4KB page with virtual address equal to physical address)
    LOG_DEBUG("Setting up identity mapping...\n");
    for (uint32_t identity_i = 0x0; identity_i < 0x100000; identity_i += 0x1000)
    {
        // for each 4KB virtual region (page), map it into a page frame
        vmm_map_page(kernel_addr_space, (void*)identity_i, (void*)identity_i, PTE_PRESENT | PTE_WRITABLE);
    }
    
    // set up physmap (map all available physical memory starting at PHYSMAP_BASE (3GB))
    uint32_t total_frames = kmm_get_total_frames();
    uint32_t max_phys_addr = total_frames * VMM_PAGE_SIZE;
    
    
    LOG_DEBUG("Setting up physmap...\n");
    for (uint32_t phys = 0; phys < max_phys_addr; phys += VMM_PAGE_SIZE)
    { 
        // convert this physical address to a virtual one
        void* virt = (void*) PHYS_TO_VIRT(phys);
        // LOG_DEBUG("Virt: 0x%08x\n", virt);

        // vmm_create_pt(kernel_addr_space, (void*)virt, PDE_PRESENT| PDE_WRITABLE);

        // map this page onto a frame
        vmm_map_page(kernel_addr_space, (void*)virt, (void*)phys, PTE_PRESENT | PTE_WRITABLE);
    }

    // THIS DOES NOT PRINT OUT!
    // switch to the newly created kernel page directory
    LOG_DEBUG("Switching to kernel space...\n");
    vmm_switch_pagedir(kernel_addr_space);

    kmm_print_status();

}

pagedir_t* vmm_create_address_space(void)
{
    // allocate a physical frame
    void* frame_phys_addr = kmm_frame_alloc();

    if (!frame_phys_addr)
        return NULL;

    // for page tables and dirs to access this frame, need to convert it into virtual address
    pagedir_t* pagedir_addr = PHYS_TO_VIRT(frame_phys_addr);

    // clear out the page directory / address space
    memset(pagedir_addr, 0, sizeof(pagedir_t));

    return pagedir_addr;
}

void vmm_map_page(pagedir_t* pdir, void* virtual, void* physical, uint32_t flags)
{
    // TODO: validate if pdir exists
    if (!pdir)
    {
        LOG_ERROR("Page directory is NULL!\n");
        return;
    }

    // ensure a page table exists for the virtual address
    // done by accessing the PD_index
    uint32_t pagedir_i = VMM_DIR_INDEX(virtual);

    pde_t pde = pdir->table[pagedir_i];

    // Create page table if needed
    if (!PDE_IS_PRESENT(pde))
    {
        vmm_create_pt(pdir, virtual, PDE_PRESENT | PDE_WRITABLE);
        
        // re-read PDE after creation
        pde = pdir->table[pagedir_i];
        
        if (!PDE_IS_PRESENT(pde))
        {
            LOG_ERROR("ERROR: Failed to create page table for virt 0x%08x\n", (uint32_t)virtual);
            return;
        }
    }

    // page table now exists, read again and extract physical address
    pde = pdir->table[pagedir_i];

    uint32_t pagetable_phys_addr = (uint32_t) PDE_PTABLE_ADDR(pde);

    // convert to virtual address for dereferencing
    pagetable_t* ptable = (pagetable_t*) PHYS_TO_VIRT(pagetable_phys_addr);

    // locate appropriate entry within the page table
    // get the page table index from original address
    uint32_t pagetable_i = VMM_TABLE_INDEX(virtual);

    // get page table entry
    pte_t* pte = &(ptable->table[pagetable_i]);

    // create mapping
    pte_t new_pte = _pte_create(physical, flags);

    // assign to entry
    *pte = new_pte;

    // TODO: flush TLB????
}

void vmm_create_pt(pagedir_t* pdir, void* virtual, uint32_t flags)
{
    // LOG_DEBUG("Inside create_pt!\n");
    // LOG_DEBUG("Inside create_pt!\n");
    // LOG_DEBUG("Inside create_pt!\n");
    // LOG_DEBUG("Inside create_pt!\n");

    if (!pdir)
    {
        // LOG_ERROR("PDIR DOES NOT EXIST!\n");
        return;
    }

    // determine which dir entry corresponding to virtual address
    uint32_t pagedir_i = VMM_DIR_INDEX(virtual);

    pde_t* pde = &(pdir->table[pagedir_i]);

    // check if page table already exists
    if (PDE_IS_PRESENT(*pde))
        return;
    
    // table does not exist
    // allocate and init new page table
    // LOG_DEBUG("Calling kmm_frame_alloc\n");
    void* table_frame_addr = kmm_frame_alloc();
    // LOG_DEBUG("Outside kmm_frame_alloc\n");
    
    // convert to virtual for memset
    void* table_frame_virt_addr = (void*) PHYS_TO_VIRT(table_frame_addr);
    
    // clear the frame
    // LOG_DEBUG("Before memset\n");
    memset(table_frame_virt_addr, 0, sizeof(pagetable_t));
    // LOG_DEBUG("After memset\n");
    
    // update page dir to reference new table
    pde_t new_entry = _pde_create(table_frame_addr, flags);

    *pde = new_entry;
}


// TODO: implement handler
void _vmm_page_fault_handler(interrupt_context_t* ctx)
{
    LOG_ERROR("PAGE FAULT!!!");

    // while (1)
    //     asm volatile ("hlt" :::);
}


pagedir_t* vmm_get_kerneldir(void)
{
    return _vmm_kernel_pagedir;
}

pagedir_t* vmm_get_current_pagedir(void)
{
    return _vmm_current_pagedir;
}

void* vmm_get_phys_frame(pagedir_t* pdir, void* virtual)
{
    // validate addresses
    if (!pdir || !virtual) 
        return NULL;


    // get page dir entry
    uint32_t pagedir_i = VMM_DIR_INDEX(virtual);

    pde_t pde = pdir->table[pagedir_i];

    // check if ptable exists
    if (!PDE_IS_PRESENT(pde))
        return NULL;


    // get page table
    uint32_t pagetable_phys_addr = (uint32_t) PDE_PTABLE_ADDR(pde);

    pagetable_t* ptable = (pagetable_t*) PHYS_TO_VIRT(pagetable_phys_addr);

    // find page table entry
    uint32_t pagetable_i = VMM_TABLE_INDEX(virtual);

    pte_t pte = ptable->table[pagetable_i];

    // check if entry is present
    if (!PTE_IS_PRESENT(pte))
        return NULL;


    // get physical frame address
    void* frame_phys_addr = (void*) PTE_FRAME_ADDR(pte);
    
    return frame_phys_addr;
}

int32_t vmm_page_alloc(pte_t* pte, uint32_t flags)
{
    // validate PTE pointer
    // if PTE is present, return 0
    if (!pte)
    {
        LOG_ERROR("vmm_page_alloc: PTE is NULL!\n");
        return -1;
    }
    
    // check if the entry already has an allocated frame
    if (PTE_IS_PRESENT(*pte))
    {
        LOG_ERROR("vmm_page_alloc: PTE already present!\n");
        return 0;
    }

    // allocate a new physical frame and create a PTE to reference it
    void* frame_physical_addr = kmm_frame_alloc();

    // check if OOM
    if (!frame_physical_addr)
    {
        LOG_ERROR("vmm_page_alloc: Out of memory!\n");
        return -1;
    }

    // create PTE for the frame
    pte_t new_entry = _pte_create(frame_physical_addr, flags);

    *pte = new_entry;

    return 0;

}

void vmm_page_free(pte_t* pte)
{
    // validate PTE
    if (!pte)
    {
        LOG_ERROR("vmm_page_free: PTE is NULL!\n");
        return;
    }

    // check if PTE does not exist
    if (!PTE_IS_PRESENT(*pte))
    {
        LOG_ERROR("vmm_page_free: PTE is not present!");
        return;
    }

    // get physical frame address
    void* frame_physical_addr = (void*)PTE_FRAME_ADDR(*pte);

    if (!frame_physical_addr)
        return;
    
    // bounds check
    if ((uintptr_t)frame_physical_addr == 0)
        return;
    
    // frame must be within valid physical memory range
    uint32_t max_phys_addr = kmm_get_total_frames() * VMM_PAGE_SIZE;

    if ((uintptr_t)frame_physical_addr >= max_phys_addr)
        return;


    // free in physical memory
    kmm_frame_free(frame_physical_addr);

    // mark as not present
    *pte = (uint32_t)PTE_FRAME_ADDR(*pte) & (uint32_t)~PTE_PRESENT;

}

bool vmm_alloc_region(pagedir_t* pdir, void* virtual, size_t size, uint32_t flags)
{
    // validate parameters
    if (!pdir || !virtual)
    {
        LOG_ERROR("vmm_alloc_region: Param(s) are NULL!\n");
        return false;
    }

    if (size == 0)
    {
        LOG_ERROR("vmm_alloc_region: Size of region is zero!\n");
        return false;
    }

    // compute starting and ending addresses
    // uintptr_t start_addr = (uintptr_t) ALIGN((uintptr_t)virtual, VMM_PAGE_SIZE);

    // this janky thing works for now
    uintptr_t start_addr = ((uintptr_t)virtual) & ~(VMM_PAGE_SIZE - 1);
    uintptr_t end_addr = (uintptr_t) ALIGN((uintptr_t)virtual + size, VMM_PAGE_SIZE);

    // iterate through addr
    for (uintptr_t addr = start_addr; addr < end_addr; addr += VMM_PAGE_SIZE)
    {
        // for each addr:
        // 1) get page table address (create if not found)
        // 2) allocate frame
        // 3) set PTE

        // get pagedir entry
        uint32_t pagedir_i = VMM_DIR_INDEX(addr);

        // get PDE
        pde_t pde = pdir->table[pagedir_i];

        // check if a page table is present
        if (!PDE_IS_PRESENT(pde))
        {
            // create a page table
            vmm_create_pt(pdir, (void*)addr, PDE_PRESENT | PDE_WRITABLE);

            // recheck pde at pagedir_i
            pde = pdir->table[pagedir_i];

            // check if allocation failed
            if (!PDE_IS_PRESENT(pde))
            {
                LOG_ERROR("vmm_alloc_region: failed to allocate page table!\n");
                return false;
            }
        }

        // from PDE, get address of ptable
        uint32_t pagetable_phys_addr = PDE_PTABLE_ADDR(pde); 

        pagetable_t* ptable = (pagetable_t*) PHYS_TO_VIRT(pagetable_phys_addr);

        // find page table entry
        uint32_t pagetable_i = VMM_TABLE_INDEX(addr);

        pte_t* pte = &(ptable->table[pagetable_i]);

        // check if this frame is already allocated
        if (PTE_IS_PRESENT(*pte))
            continue;   // skip


        // allocate a frame
        void* frame_physical_addr = kmm_frame_alloc();

        // validate
        if (!frame_physical_addr)
        {
            LOG_ERROR("vmm_alloc_region: Alloc faliure!\n");
            return false;
        }

        // assign to ptable
        pte_t new_entry = _pte_create(frame_physical_addr, flags);

        *pte = new_entry;

    }

    // great success
    return true;

}

bool vmm_free_region(pagedir_t* pdir, void* virtual, size_t size)
{
    // built on vibes...

    // validate parameters
    if (!pdir || !virtual)
    {
        LOG_ERROR("vmm_free_region: Invalid parameter(s)!\n");
        return false;
    }

    if (size == 0)
    {
        LOG_ERROR("vmm_free_region: Size is zero!\n");
        return false;
    }

    // compute start and end addr
    uintptr_t start_addr = (uintptr_t) ALIGN((uintptr_t)virtual, VMM_PAGE_SIZE);
    uintptr_t end_addr = (uintptr_t) ALIGN((uintptr_t)virtual + size, VMM_PAGE_SIZE);

    uint32_t start_pd_index = VMM_DIR_INDEX(start_addr);
    uint32_t end_pd_index = VMM_DIR_INDEX(end_addr - 1);

    // iterate through each page in the region
    for (uintptr_t addr = start_addr; addr < end_addr; addr += VMM_PAGE_SIZE)
    {
        // get page directory index
        uint32_t pagedir_i = VMM_DIR_INDEX(addr);
        pde_t pde = pdir->table[pagedir_i];

        // skip if page table doesn't exist
        if (!PDE_IS_PRESENT(pde))
            continue;

        // get the page table
        uint32_t pagetable_phys_addr = PDE_PTABLE_ADDR(pde);
        pagetable_t* ptable = (pagetable_t*)PHYS_TO_VIRT(pagetable_phys_addr);

        // get page table index
        uint32_t pagetable_i = VMM_TABLE_INDEX(addr);
        pte_t* pte = &(ptable->table[pagetable_i]);

        // check if page is present
        if (!PTE_IS_PRESENT(*pte))
            continue;   // skip

        // get physical frame address
        void* frame_phys = (void*)PTE_FRAME_ADDR(*pte);

        // free the frame
        if (frame_phys)
            kmm_frame_free(frame_phys);

        // clear the page table entry
        *pte = 0;

        // invalidate TLB entry
        flush_tlb((void*)addr);
    }

    // check for empty page tables and free them
    bool freed_any_pt = false;
    for (uint32_t pd_index = start_pd_index; pd_index <= end_pd_index; pd_index++)
    {
        pde_t pde = pdir->table[pd_index];

        // skip if page table doesn't exist
        if (!PDE_IS_PRESENT(pde))
            continue;

        // get the page table
        uint32_t pagetable_phys_addr = PDE_PTABLE_ADDR(pde);
        pagetable_t* ptable = (pagetable_t*) PHYS_TO_VIRT(pagetable_phys_addr);

        // check if page table is completely empty
        bool is_empty = true;
        for (uint32_t i = 0; i < VMM_PAGES_PER_TABLE; i++)
        {
            if (PTE_IS_PRESENT(ptable->table[i]))
            {
                is_empty = false;
                break;
            }
        }

        // if empty, free the page table
        if (is_empty)
        {
            LOG_DEBUG("vmm_free_region: Freeing empty page table at PD index %u\n", pd_index);

            // free page table frame
            kmm_frame_free((void*)pagetable_phys_addr);

            // clear the page directory entry
            pdir->table[pd_index] = 0;
            freed_any_pt = true;
        }
    }

    // great success
    return true;
}

pagetable_t* vmm_clone_pagetable(pagetable_t* src)
{
    // check null
    if (!src)
        return NULL;

    // must be page-aligned (4KB)
    uintptr_t src_virt = (uintptr_t)src;

    if (src_virt & (VMM_PAGE_SIZE - 1))
        return NULL;

    // page tables must be accessed via physmap
    if (src_virt < PHYSMAP_BASE)
        return NULL;

    // check bounds of physical frame
    uintptr_t src_physical_addr = (uintptr_t)VIRT_TO_PHYS(src);
    if (src_physical_addr >= kmm_get_total_frames() * VMM_PAGE_SIZE);
        return NULL;


    // allocate a new physical frame
    void* new_frame_phys_addr = kmm_frame_alloc();

    if (!new_frame_phys_addr)
    {
        LOG_ERROR("vmm_clone_pagetable: Alloc failed! (ptable)\n");
        return NULL;
    }

    // convert to virtual
    pagetable_t* cloned_ptable = (pagetable_t*) PHYS_TO_VIRT(new_frame_phys_addr);

    // clear it
    memset(cloned_ptable, 0, sizeof(pagetable_t));

    // iterate over all pages in the table
    for (uint32_t page = 0; page < VMM_PAGES_PER_TABLE; page++)
    {
        // get each entry in src
        pte_t pte = src->table[page];

        // check if present
        if (!PTE_IS_PRESENT(pte))
            continue;   // skip

        // allocate a new physical frame for the data
        void* new_page_phys_addr = kmm_frame_alloc();

        if (!new_page_phys_addr)
        {
            LOG_ERROR("vmm_clone_pagetable: Alloc failed! (page)\n");
            break;  // stop cloning
        }

        // get original flags
        uint32_t src_flags = PTE_FLAGS(pte);

        // get virtuals
        void* src_virt_addr = PHYS_TO_VIRT((void*)PTE_FRAME_ADDR(pte));
        void* dst_virt_addr = PHYS_TO_VIRT(new_frame_phys_addr);

        // copy
        memcpy(dst_virt_addr, src_virt_addr, VMM_PAGE_SIZE);

        // create new entry for cloned table (same flags)
        pte_t new_entry = _pte_create(new_page_phys_addr, src_flags);

        cloned_ptable->table[page] = new_entry;
    }

    return cloned_ptable;
}

// ...existing code...
pagedir_t* vmm_clone_pagedir(void)
{
    // built on vibes...

    // get current and kernel pagedirs
    // mutuallu exclusive options
    pagedir_t* curr = vmm_get_current_pagedir();
    pagedir_t* kdir = vmm_get_kerneldir();

    if (!curr)
        return NULL;

    // create a new empty page directory
    pagedir_t* newdir = vmm_create_address_space();

    if (!newdir)
        return NULL;

    // iterate through all PDEs
    for (uint32_t i = 0; i < 1024; i++)
    {
        pde_t src_pde = curr->table[i];

        // skip not-present entries
        if (!PDE_IS_PRESENT(src_pde))
            continue;

        // if kernel mapping, shallow copy page table
        if (PDE_IS_PRESENT(kdir->table[i]) && PDE_PTABLE_ADDR(kdir->table[i]) == PDE_PTABLE_ADDR(src_pde))
            // copy src pde
            newdir->table[i] = src_pde;

        else
        {
            // user mapping branch
            uint32_t src_pt_phys = PDE_PTABLE_ADDR(src_pde);
            pagetable_t* src_pt = (pagetable_t*)PHYS_TO_VIRT(src_pt_phys);

            // clone page table
            pagetable_t* cloned_pt = vmm_clone_pagetable(src_pt);

            if (!cloned_pt)
            {
                LOG_ERROR("vmm_clone_pagedir: failed to clone PT at PDE %u\n", i);
                return NULL;
            }

            // preserve flags
            uint32_t pde_flags = PDE_FLAGS(src_pde);
            pde_t new_pde = _pde_create(VIRT_TO_PHYS(cloned_pt), pde_flags);

            // assign new pde
            newdir->table[i] = new_pde;
        }
    }

    return newdir;
}


// helpers
bool vmm_switch_pagedir(pagedir_t* new_pagedir)
{
    // TODO: needs updates
    
    // validate pagedir
    if (!new_pagedir)
        return false;
    

    uint32_t pagedir_phys_addr = (uint32_t) VIRT_TO_PHYS(new_pagedir);

    // store in %CR3
    asm volatile ("mov %0, %%cr3" :: "r"(pagedir_phys_addr));

    // set global state
    _vmm_current_pagedir = new_pagedir;

    return true;
}

void vmm_read_cr3(void)
{
    uint32_t cr3_value;

    // get %CR3 value
    asm volatile ("mov %%cr3 , %0" : "=r"(cr3_value));

    // set global state
    _vmm_current_pagedir = (pagedir_t*) PHYS_TO_VIRT(cr3_value);
    
}

static inline void flush_tlb(void* virt)
{
    asm volatile("invlpg (%0)" :: "r"(virt) : "memory");
}


#endif
