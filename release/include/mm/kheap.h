#ifndef _KHEAP_H
#define _KHEAP_H

//*****************************************************************************
//*
//*  @file		[kheap.h]
//*  @author    
//*  @brief		Kernel Heap Management Header
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

typedef struct _free_block_hdr free_block_hdr;

// the data structure represents a generic heap arena
struct __heap_descriptor {
	
	void* 			  state;		  // ptr to internal state
	uintptr_t         start;          // start address of the heap
	uintptr_t         end;            // end address of the heap
	uint32_t          max_size;       // maximum size of the heap
	uint8_t           is_supervisor;  // if the heap is supervisor only
	uint8_t           is_readonly;    // if the heap is read-only

};

typedef struct __heap_descriptor heap_t;

//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************

#endif // !_KHEAP_H