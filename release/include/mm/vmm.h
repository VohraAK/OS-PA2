#ifndef _VMM_H
#define _VMM_H
//*****************************************************************************
//*
//*  @file		vmm.h
//*  @author    
//*  @brief	    Defines the interface for the virtual memory manager (VMM).
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <mm/pde.h>
#include <mm/pte.h>

#include <mm/kmm.h>
#include <interrupts.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

#define VMM_PAGE_SIZE           4096    //! 4KB page size
#define VMM_PAGES_PER_TABLE     1024    //! 1024 entries per page table
#define VMM_PAGES_PER_DIR       1024    //! 1024 entries per page directory

// get page directory index (first 10 bits)
#define VMM_DIR_INDEX(addr)     (((uintptr_t)(addr) >> 22) & 0x3FF)

// get page table index (next 10 bits)
#define VMM_TABLE_INDEX(addr)   (((uintptr_t)(addr) >> 12) & 0x3FF)

// get page table offset (last 12 bits)
#define VMM_PAGE_OFFSET(addr)   ((uintptr_t)(addr) & 0xFFF)

// Page Fault Interrupt vector
#define PAGE_FAULT_INTERRUPT (uint8_t)14


// 32bit PTE/PDE entry:
// 1) 11-0 bits -> flags/control-bits
// 2) 31-12 bits -> physical address

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

//! represents a single page table in memory
typedef struct {

    //! array of page table entries 
    pte_t       table[ VMM_PAGES_PER_TABLE ];

} pagetable_t;

//! represents a page directory
typedef struct {

    //! array of page directory entries
    pde_t       table[ VMM_PAGES_PER_DIR ];

} pagedir_t;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
void vmm_init(void);
void _vmm_page_fault_handler(interrupt_context_t* ctx);
pagedir_t* vmm_create_address_space(void);
void vmm_create_pt(pagedir_t* pdir, void* virtual, uint32_t flags);
void vmm_map_page(pagedir_t* pdir, void* virtual, void* physical, uint32_t flags);
pagedir_t* vmm_get_kerneldir(void);
pagedir_t* vmm_get_current_pagedir(void);
void* vmm_get_phys_frame(pagedir_t* pdir, void* virtual);
int32_t vmm_page_alloc(pte_t* pte, uint32_t flags);
void vmm_page_free(pte_t* pte);
bool vmm_alloc_region(pagedir_t* pdir, void* virtual, size_t size, uint32_t flags);
bool vmm_free_region(pagedir_t* pdir, void* virtual, size_t size);
pagetable_t* vmm_clone_pagetable(pagetable_t* src);
pagedir_t* vmm_clone_pagedir(void);


// helpers
bool vmm_switch_pagedir(pagedir_t* pagedir);
void vmm_read_cr3(void);
static inline void flush_tlb(void* virt);

//*****************************************************************************
//**
//** 	END vmm.h
//**
//*****************************************************************************

#endif // _VMM_H