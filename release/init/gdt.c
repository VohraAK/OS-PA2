#include <init/gdt.h>
#include <stdint.h>
#include <string.h>

//! define access bits for the user mode segments

/* user code segment */
#define GDT_ACCESS_USER_CODE   \
        ((GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_TYPE |\
            GDT_ACCESS_CODE | GDT_ACCESS_READABLE))
    
/* user data segment */
#define GDT_ACCESS_USER_DATA   \
        ((GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_TYPE |\
            GDT_ACCESS_DATA | GDT_ACCESS_READWRITE))

//! define access bits for the kernel mode segments

/* kernel code is DPL0 */
#define GDT_ACCESS_KERNEL_CODE \
        ((GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_TYPE |\
            GDT_ACCESS_CODE | GDT_ACCESS_READABLE))

/* kernel data */
#define GDT_ACCESS_KERNEL_DATA ((GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 |\
            GDT_ACCESS_TYPE | GDT_ACCESS_DATA | GDT_ACCESS_READWRITE))

// standard granularity flags
#define GDT_GRANULARITY       0xCF
#define TSS_GRANULARITY       0x00 // TSS does not use granularity

//! defines for the number and index of specific entries
#define GDT_ENTRIES           6
#define GDT_NULL_ENTRY        0
#define GDT_KERNEL_CODE_ENTRY 1
#define GDT_KERNEL_DATA_ENTRY 2
#define GDT_USER_CODE_ENTRY   3
#define GDT_USER_DATA_ENTRY   4
#define GDT_TSS_ENTRY         5

//! Returns the offset of a GDT entry in the GDT array 
#define GDT_SEG_OFFSET(entry) (entry * sizeof(gdt_entry_t))

//! the global descriptor table will be created with 5 segment descriptor
//! entries and 1 TSS entry. provides separation between user and kernel mode
//! and provides a TSS to enable software task switching.
static gdt_entry_t         gdt[GDT_ENTRIES];
static gdt_ptr_t           gdt_ptr;    // Pointer to the GDT

// Function to create/initialize a GDT entry with correct parameters
void create_gdt_entry(gdt_entry_t *entry, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t granularity)
{

    entry->base_low    = (base & 0xFFFF);     // Lower 16 bits of base address
    entry->base_middle = (base >> 16) & 0xFF; // Next 8 bits of base address
    entry->base_high   = (base >> 24) & 0xFF; // Upper 8 bits of base address

    entry->limit_low    = (limit & 0xFFFF);       // Lower 16 bits of the limit
    entry->granularity  = ((limit >> 16) & 0x0F); // Upper 4 bits of the limit
    entry->granularity |= (granularity & 0xF0);   // Set granularity bits 
    entry->access       = access;

}

// Sets up the GDT with a flat protected mode setup (user mode and kernel mode)
void gdt_init_flat_protected()
{
    // Set the GDT pointer
    gdt_ptr.limit = 
            (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1; // size of GDT - 1
    gdt_ptr.base  = (uint32_t)&gdt; // Base address of the GDT

    // protected mode memory start and end
#define PR_MEM_START 0x0
#define PR_MEM_END   0xFFFFFFFF     // 4GB

    // Create GDT entries
    //! Null segment
    create_gdt_entry(&gdt[ GDT_NULL_ENTRY ], 0, 0, 0, 0);

    //! Kernel code segment
    create_gdt_entry(&gdt[ GDT_KERNEL_CODE_ENTRY ], PR_MEM_START, PR_MEM_END,
        GDT_ACCESS_KERNEL_CODE, GDT_GRANULARITY);

    //! Kernel data segment
    create_gdt_entry(&gdt[ GDT_KERNEL_DATA_ENTRY ], PR_MEM_START, PR_MEM_END,
        GDT_ACCESS_KERNEL_DATA, GDT_GRANULARITY);

    //! User code segment
    create_gdt_entry(&gdt[ GDT_USER_CODE_ENTRY ], PR_MEM_START, PR_MEM_END,
        GDT_ACCESS_USER_CODE, GDT_GRANULARITY);

    //! User data segment
    create_gdt_entry(&gdt[ GDT_USER_DATA_ENTRY ], PR_MEM_START, PR_MEM_END,
        GDT_ACCESS_USER_DATA,   GDT_GRANULARITY);
        

}

gdt_ptr_t* get_gdt_ptr(void)
{
    return &gdt_ptr; // Return the pointer to the GDT pointer structure
}