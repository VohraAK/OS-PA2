#ifndef _MM_PDE_H
#define _MM_PDE_H

#include <stdint.h>

// Page Directory Entry (PDE) flags
#define PDE_PRESENT         0x001       // Page is present in memory
#define PDE_WRITABLE        0x002       // Page is writable
#define PDE_USER            0x004       // Page is accessible in user mode
#define PDE_WRITETHROUGH    0x008       // Page Write-Through
#define PDE_CACHEDISABLE    0x010       // Page Cache Disable
#define PDE_ACCESSED        0x020       // Page has been accessed
#define PDE_DIRTY           0x040       // Page has been written to
#define PDE_SIZE_4MB        0x080       // Page size is 4MB
#define PDE_GLOBAL          0x100       // Page is global (not flushed on context switch)
#define PDE_LV4_GLOBAL      0x200       // Level 4 global page
#define PDE_FRAME_MASK      0xFFFFF000  // Mask for the frame address in the PDE

// Each Page Directory Entry is 32 bits wide, so we can represent it as a 32-bit unsigned integer
typedef uint32_t pde_t;

// Using a similar interface as PTEs for creating and manipulating PDEs
#define PDE_PTABLE_ADDR(pde)    ((pde) & PDE_FRAME_MASK)   // Get the page table address
#define PDE_FLAGS(pde)         ((pde) & ~PDE_FRAME_MASK)  // Get the flags from a PDE

#define PDE_IS_WRITABLE(pde)   ((pde) & PDE_WRITABLE)     // Check if the page is writable
#define PDE_IS_PRESENT(pde)    ((pde) & PDE_PRESENT)      // Check if the page is present
#define PDE_IS_DIRTY(pde)      ((pde) & PDE_DIRTY)        // Check if the page is dirty
#define PDE_IS_USER(pde)       ((pde) & PDE_USER)         // Check if the page is user accessible
#define PDE_IS_4MB(pde)        ((pde) & PDE_SIZE_4MB)     // Check if the page is 4MB


static inline pde_t _pde_create(void* phys_addr, uint32_t flags)
{
    // create first 20bit address part
    pde_t addr_part = (pde_t)phys_addr & PDE_FRAME_MASK;

    // create last 12bit flags part
    pde_t flags_part = flags & ~PDE_FRAME_MASK;

    // combination
    return addr_part | flags_part;
}


#endif // _MM_PDE_H