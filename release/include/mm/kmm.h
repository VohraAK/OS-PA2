#ifndef _KMM_H
#define _KMM_H
//*****************************************************************************
//*
//*  @file		kmm.h
//*  @author    
//*  @brief	    Kernel physical memory manager (KMM).
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

/* Memory map locations */
#define MEM_SIZE_LOC            0x3000
#define MEM_MAP_ENTRY_COUNT_LOC 0x3008
#define MEM_MAP_LOC             0x300C

/* A good idea to manage the physical memory in the size of page frames. so we
    can enable paging with this allocator too */
#define _KMM_BLOCK_SIZE         4096
#define _KMM_BLOCK_ALIGNMENT    _KMM_BLOCK_SIZE

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

/* The int 0x15 eax=0xe820 memory map is used to describe the physical memory
    layout of the system. entries are stored somewhere in the memory during 
    bootsector. The entries are then access by the kernel to determine the
    available memory regions. Available memory region types are:
    1 = usable, 2 = reserved, 3 = ACPI reclaimable, 4 = ACPI NVS, 5 = bad memory
    */
typedef struct {

    uint32_t    baseLow;
    uint32_t    baseHigh;
    uint32_t    lengthLow; 
    uint32_t    lengthHigh;
    uint32_t    type;
    uint32_t    ACPI;

} e820_entry_t;

/* The int 0x15 eax=0xe801 memory size is used to describe the physical memory
 size of the system. It is used to determine the total amount of memory
 available in the system. */
typedef struct {

    uint32_t    memLow;       //! number of KB blocks between 1MB and 16MB
    uint32_t    memHigh;      //! number of 64K blocks above 16MB

} e801_memsize_t;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------


#endif // !_KMM_H
