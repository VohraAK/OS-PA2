#ifndef _VMM_C
#define _VMM_C

#include <mm/vmm.h>

// global state variables
static pagedir_t* _vmm_current_pagedir = NULL;
static pagedir_t* _vmm_kernel_pagedir = NULL;

// NOTE: page directories and tables MUST BE accessed through physmap

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



#endif
