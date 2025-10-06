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

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

#define VMM_PAGE_SIZE           4096    //! 4KB page size
#define VMM_PAGES_PER_TABLE     1024    //! 1024 entries per page table
#define VMM_PAGES_PER_DIR       1024    //! 1024 entries per page directory

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


//*****************************************************************************
//**
//** 	END vmm.h
//**
//*****************************************************************************

#endif // _VMM_H