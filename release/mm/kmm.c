#ifndef _KMM_C
#define _KMM_C

#include <mm/kmm.h>

// code start and end files
extern uint32_t kernel_start;
extern uint32_t kernel_end;

static uint32_t available_size = 0;
static uint32_t mem_map_entries_count = 0;
static e820_entry_t* mem_map;
static uint32_t* bitmap;
static uint32_t bitmap_size = 0;

// status variables (for debugging)
static uint32_t free_frames;
static uint32_t used_frames;


// helpers
void kmm_get_available_mem()
{
    // BIOS dumps memory size onto MEM_SIZE_LOC -> read this in
    e801_memsize_t* mem_size = (e801_memsize_t*) MEM_SIZE_LOC;

    // calculate and return available memory (in KB)
    available_size = AVLBL_MEM(mem_size->memLow, mem_size->memHigh);
}

void kmm_get_physical_mem_map()
{
    // BIOS dumps the array onto MEM_MAP_LOC -> read it in
    mem_map_entries_count = *(uint32_t*) MEM_MAP_ENTRY_COUNT_LOC;
    mem_map = (e820_entry_t*) MEM_MAP_LOC;
}

void kmm_print_status(void)
{
    LOG_DEBUG("Total memory: %u KB\n", available_size);
    LOG_DEBUG("Used frames: %u\n", used_frames);
    LOG_DEBUG("Free frames: %u\n", free_frames);
    
}

bitmap_frame_info_t kmm_get_first_free_bit(void)
{
    // a bit naive, but oh well...
    bitmap_frame_info_t free_frame_info;

    // set with sentinel values
    free_frame_info.index = (uint32_t) 0xFFFFFFFF;
    free_frame_info.offset = (uint32_t) 0xFFFFFFFF;

    uint32_t total_frames = kmm_get_total_frames();

    // search bitmap for first free bit -> linear search? -> may go binary search later
    for (uint32_t i = 0; i < bitmap_size; i++)
    {
        // iterate through each 32-bit field
        uint32_t field = bitmap[i];

        // check if this field is fully set
        if (field == (uint32_t)0xFFFFFFFF)
            continue;

        // check each bit
        for (uint32_t bit_i = 0; bit_i < 32; bit_i++)
        {
            uint32_t frame = i * 32u + bit_i;

            // check if bit_i is valid (RECHECK)
            if (frame >= (total_frames))
                break;

            // check if bit is set
            if ((field & (1u << bit_i)) == 0)
            {
                // free frame
                free_frame_info.index = i;
                free_frame_info.offset = bit_i;

                return free_frame_info;
            }
        }
    }

    // no free frames
    return free_frame_info;
}


// API definitions
uint32_t kmm_get_total_frames(void)
{
    return (used_frames + free_frames);
}

uint32_t kmm_get_used_frames(void)
{
    return used_frames;
}

void kmm_init(void)
{
    LOG_DEBUG("------------------------------\n");
    LOG_DEBUG("KMM INIT\n");

    kmm_get_available_mem();
    kmm_get_physical_mem_map();

    // get physical addressesof kernel start and end
    uint32_t kstart_addr = (uint32_t) VIRT_TO_PHYS(&kernel_start);
    uint32_t kend_addr = (uint32_t) VIRT_TO_PHYS(&kernel_end);

    // calculate bitmap_size (with ceil) -> this is the number of 32bit entries required
    uint32_t pageframe_total = (available_size * 1024) / _KMM_BLOCK_SIZE;

    // set number of frames (total number of bits)
    used_frames = pageframe_total;
    free_frames = 0;
    
    bitmap_size = (pageframe_total + 31) / 32;

    // compute start-of-bitmap address (aligned)
    uint32_t bitmap_start_addr = (uint32_t) ALIGN(kend_addr, _KMM_BLOCK_ALIGNMENT);
    
    // compute end-of-bitmap address (unaligned)
    uint32_t bitmap_end_addr = bitmap_start_addr + (bitmap_size * sizeof(uint32_t));

    // ensure the bitmap end is block-aligned so reservation covers full frames
    bitmap_end_addr = (uint32_t) ALIGN(bitmap_end_addr, _KMM_BLOCK_ALIGNMENT);

    // place bitmap right after kernel code and data (kernel_end)
    bitmap = (uint32_t*) PHYS_TO_VIRT(bitmap_start_addr);

    // initialise bitmap
    // total space occupied (in bytes)
    memset(bitmap, (uint8_t)0xFF, bitmap_size * sizeof(uint32_t));



    // iterate through the mem_map
    for (uint32_t i = 0; i < mem_map_entries_count; i++)
    {
        // get current region
        e820_entry_t* region = &mem_map[i];

        // skip if unusable
        if (region->type != 1)
            continue;

        // get base and lengths
        uint32_t base_low = region->baseLow;
        uint32_t base_high = region->baseHigh;
        uint32_t length_low = region->lengthLow;
        uint32_t length_high = region->lengthHigh;

        // get region type
        uint32_t type = region->type;

        // merge base and lengths into a 64-bit field
        uint64_t base_addr = ((uint64_t)base_high << 32) | ((uint64_t)base_low);
        uint64_t length = ((uint64_t)length_high << 32) | ((uint64_t)length_low);

        // compute start and end points of the region, rounding up and down for alighment
        uint64_t region_start = (uint64_t)ALIGN(base_addr, _KMM_BLOCK_ALIGNMENT);
        uint64_t region_end = (uint64_t)ALIGN(base_addr + length, _KMM_BLOCK_ALIGNMENT);

        // from the endpoints, get the starting and ending frame number
        uint32_t starting_frame = (region_start) / (_KMM_BLOCK_SIZE);
        uint32_t ending_frame = (region_end) / (_KMM_BLOCK_SIZE);

        // iterate over each frame region, conditionally setting each frame bit
        for (uint32_t frame = starting_frame; frame < ending_frame; frame++)
        {
            // I need some sort of index to access a specific 32-bit enty in the bitmap
            uint32_t index = (frame) / (32);

            // I also need an offset to access the particular bit
            uint32_t offset = (frame) % 32;

            // mark as free
            bitmap[index] &= ~(1u << offset);

            // increment free count
            free_frames += 1;

            // decrement used count
            used_frames -= 1;
        }

    }


    // reserve low memory regions (640KiB instead of till 1MB for ... reasons ...)
    kmm_setup_memory_region(0x00000000, 0x000A0000, true);


    // reserve kernel frames
    uint32_t kernel_start_frame = (uint32_t)ALIGN(kstart_addr, _KMM_BLOCK_ALIGNMENT);
    kernel_start_frame /= _KMM_BLOCK_SIZE;

    uint32_t kernel_end_frame = (uint32_t)ALIGN(kend_addr, _KMM_BLOCK_ALIGNMENT);
    kernel_end_frame /= _KMM_BLOCK_SIZE;
    
    for (uint32_t frame = kernel_start_frame; frame < kernel_end_frame; frame++)
    {
        uint32_t index = frame / 32;
        uint32_t offset = frame % 32;
        
        // only reserve if currently marked free (avoid double-counting)
        if ((bitmap[index] & (1u << offset)) == 0)
        {
            // mark as used
            bitmap[index] |= (1u << offset);

            free_frames -= 1;
            used_frames += 1;
        }
    }
    
    // reserve bitmap frames
    uint32_t bitmap_start_frame = bitmap_start_addr / _KMM_BLOCK_SIZE;
    uint32_t bitmap_end_frame = bitmap_end_addr / _KMM_BLOCK_SIZE;
    
    for (uint32_t frame = bitmap_start_frame; frame < bitmap_end_frame; frame++)
    {
        uint32_t index = frame / 32;
        uint32_t offset = frame % 32;
        
        // only reserve if currently marked free (avoid double-counting)
        if ((bitmap[index] & (1u << offset)) == 0)
        {
            // mark as used
            bitmap[index] |= (1u << offset);

            free_frames -= 1;
            used_frames += 1;
        }
    }

    // reserve frame 0
    if ((bitmap[0] & 0x1u) == 0) 
    {  
        // mark as used
        bitmap[0] |= 0x1u;
        
        free_frames -= 1;
        used_frames += 1;
    }


    // TODO: print debug info
    kmm_print_status();
}

void kmm_setup_memory_region(uint32_t base, uint32_t size, bool is_reserved)
{
    // don't modify the boot/reserved low-memory area when clearing regions
    // const uint32_t reserved_end = 0x100000;

    // if (!is_reserved)
    // {
    //     // If the entire region lies below reserved_end, nothing to do
    //     if ((uint64_t)base + (uint64_t)size <= (uint64_t)reserved_end)
    //         return;

    //     // If region partially overlaps reserved area, adjust to skip the reserved portion
    //     if (base < reserved_end)
    //     {
    //         uint32_t overlap = reserved_end - base;
    //         base += overlap;
    //         size -= overlap;
    //     }
    // }

    // convert offsets into a physical region
    uint64_t region_start = (uint64_t)ALIGN(base, _KMM_BLOCK_ALIGNMENT);
    uint64_t region_end = (uint64_t)ALIGN(base + size, _KMM_BLOCK_ALIGNMENT);

    uint32_t starting_frame = (region_start) / (_KMM_BLOCK_SIZE);
    uint32_t ending_frame = (region_end) / (_KMM_BLOCK_SIZE);

    for (uint32_t frame = starting_frame; frame < ending_frame; frame++)
    {
        uint32_t index = (frame) / (32);
        uint32_t offset = (frame) % (32);

        // conditionally set frames
        if (is_reserved)
        {
            // only mark if currently free
            if ((bitmap[index] & (1u << offset)) == 0) 
            {
                bitmap[index] |= (1 << offset);
                free_frames -= 1;
                used_frames += 1;
            }
        }

        else
        {
            // only free if currently used
            if ((bitmap[index] & (1u << offset)) != 0) 
            {
                bitmap[index] &= ~(1 << offset);
                free_frames += 1;
                used_frames -= 1;
            }
        }

    }

}

void* kmm_frame_alloc(void)
{
    // find first free frame
    bitmap_frame_info_t free_frame_info = kmm_get_first_free_bit();

    // no free frames
    if (free_frame_info.index == (uint32_t) 0xFFFFFFFF)
        return NULL;

    uint32_t frame_number = (free_frame_info.index * 32u) + free_frame_info.offset;

    // // never return frame 0; if encountered, reserve it and continue searching
    // if (frame_number == 0)
    // {
    //     // mark as used if not already
    //     if ((bitmap[free_frame_info.index] & (1u << free_frame_info.offset)) == 0) 
    //     {
    //         bitmap[free_frame_info.index] |= (1u << free_frame_info.offset);
    //         used_frames += 1;
    //         free_frames -= 1;
    //     }
    // }

    // mark that frame as used
    bitmap[free_frame_info.index] |= (1u << free_frame_info.offset);

    // update the usage counter
    used_frames += 1;

    // update the free counter
    free_frames -= 1;
        
    // translate frame_number to physical address
    void* physical_addr = (void*) (frame_number * _KMM_BLOCK_SIZE);

    return physical_addr;

}   

void kmm_frame_free(void* phys_addr)
{
    // validate physical address
    if (phys_addr == NULL)
        return;

    uint32_t addr = (uint32_t) phys_addr;
    uint32_t total_frames = kmm_get_total_frames();

    // check if physical address is page aligned
    if ((addr % _KMM_BLOCK_ALIGNMENT) != 0)
        return;

    // convert to frame index
    uint32_t frame_number = addr / (_KMM_BLOCK_SIZE);

    // validate frame index
    if (frame_number >= total_frames)
        return;

    // check if frame 0 is being accessed
    if (frame_number == 0)
        return;

    // check (not reserved, bitmap, kernel region)
    uint32_t reserved_end = 0x100000;
    uint32_t reserved_end_frame = (reserved_end) / (_KMM_BLOCK_SIZE);

    if (frame_number < reserved_end_frame)
        return;

    // kernel check
    uint32_t kernel_start_addr = (uint32_t) VIRT_TO_PHYS(&kernel_start);
    uint32_t kernel_end_addr = (uint32_t) VIRT_TO_PHYS(&kernel_end);

    uint32_t kernel_start_frame = (uint32_t)ALIGN(kernel_start_addr, _KMM_BLOCK_ALIGNMENT);
    kernel_start_frame /= _KMM_BLOCK_SIZE;

    uint32_t kernel_end_frame = (uint32_t)ALIGN(kernel_end_addr, _KMM_BLOCK_ALIGNMENT);
    kernel_end_frame /= _KMM_BLOCK_SIZE;

    if (frame_number >= kernel_start_frame && frame_number < kernel_end_frame)
        return;

    // bitmap check (had to recompute start addr)
    uint32_t bitmap_start_addr = (uint32_t)ALIGN(kernel_end_addr, _KMM_BLOCK_ALIGNMENT);

    uint32_t bitmap_end_addr = bitmap_start_addr + (bitmap_size * sizeof(uint32_t));
    bitmap_end_addr = (uint32_t)ALIGN(bitmap_end_addr, _KMM_BLOCK_ALIGNMENT);

    uint32_t bitmap_start_frame = bitmap_start_addr / _KMM_BLOCK_SIZE;
    uint32_t bitmap_end_frame = bitmap_end_addr / _KMM_BLOCK_SIZE;

    if (frame_number >= bitmap_start_frame && frame_number < bitmap_end_frame)
        return;

    // clear corresponding bit
    uint32_t index = frame_number / 32;
    uint32_t offset = frame_number % 32;

    // check if already free
    if ((bitmap[index] & (1u << offset)) == 0)
        return;

    // free the frame
    bitmap[index] &= ~(1u << offset);

    // update counters
    free_frames += 1;
    used_frames -= 1;

}

#endif