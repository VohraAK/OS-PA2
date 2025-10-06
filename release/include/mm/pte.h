#ifndef _MM_PTE_H
#define _MM_PTE_H

#include <stdint.h>

// Page Table Entry (PTE) flags
#define PTE_PRESENT         0x001 // Page is present in memory
#define PTE_WRITABLE        0x002 // Page is writable
#define PTE_USER            0x004 // Page is accessible in user mode
#define PTE_WRITETHROUGH    0x008 // Page Write-Through
#define PTE_CACHEDISABLE    0x010 // Page Cache Disable
#define PTE_ACCESSED        0x020 // Page has been accessed
#define PTE_DIRTY           0x040 // Page has been written to
#define PTE_PAT             0x080
#define PTE_GLOBAL          0x100 // Page is global (not flushed on context switch)
#define PTE_LV4_GLOBAL      0x200

#define PTE_FRAME_MASK      0xFFFFF000 // Mask for the frame address in the PTE

// a page table entry is 32 bits wide, so we can represent it as a 32-bit unsigned integer
typedef uint32_t pte_t;

// utils to create and manipulate PTEs
#define PTE_FRAME_ADDR(pte)    ((pte) & PTE_FRAME_MASK)   // Get the frame address from a PTE
#define PTE_FLAGS(pte)         ((pte) & ~PTE_FRAME_MASK)  // Get the flags from a PTE

#define PTE_IS_PRESENT(pte)    ((pte) & PTE_PRESENT)        // Check if the page is present
#define PTE_IS_WRITABLE(pte)   ((pte) & PTE_WRITABLE)       // Check if the page is writable
#define PTE_IS_DIRTY(pte)      ((pte) & PTE_DIRTY)          // Check if the page is dirty

#define PTE_SET_PRESENT(pte)   ((pte) |= PTE_PRESENT)          // Set the present flag
#define PTE_UNSET_PRESENT(pte) ((pte) &= ~PTE_PRESENT)         // Unset the present flag

#define PTE_SET_WRITABLE(pte)  ((pte) |= PTE_WRITABLE)         // Set the writable flag
#define PTE_UNSET_WRITABLE(pte) ((pte) &= ~PTE_WRITABLE)       // Unset the writable flag

#define PTE_SET_DIRTY(pte)     ((pte) |= PTE_DIRTY)            // Set the dirty flag
#define PTE_UNSET_DIRTY(pte)   ((pte) &= ~PTE_DIRTY)          // Unset the dirty flag


#endif // _MM_PTE_H