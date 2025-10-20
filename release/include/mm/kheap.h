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
#define ALLOC_BLOCK_HDR_SIZE (sizeof(alloc_block_hdr))	
#define BUDDY_MIN_ORDER 5	// 32B
#define BUDDY_MAX_ORDER 32
#define MAGIC_NO 0xfeedbeef
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

// allocated-block header
typedef struct _alloc_block_header 
{
	size_t size;	// bytes requested
	uint32_t magic;	// sentinel value

} alloc_block_hdr;


// free-block header placed at free block base
typedef struct _free_block_hdr {

    struct _free_block_hdr* prev;
    struct _free_block_hdr* next;

} free_block_hdr;


// per-heap buddy state (maintain heap base, size, min and max block orders, free lists per order)
typedef struct _kheap_state {

    uintptr_t       base;
    size_t          size;
    unsigned        min_order;                  
    unsigned        max_order;                  
    free_block_hdr* free_lists[BUDDY_MAX_ORDER];

} kheap_state_t;


//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
void  kheap_init(heap_t *heap, void *start, size_t size, size_t max_size, bool is_supervisor, bool is_readonly);
void* kmalloc(heap_t *heap, size_t size);
void  kfree(heap_t *heap, void* ptr);
void* krealloc(heap_t *heap, void *ptr, size_t new_size);


// create helpers for allocator math and free list management, for linked-list operations, alignments

// helpers / API
uint32_t _compute_highest_exponent(size_t n);
heap_t* get_kernel_heap(void);
void free(void* ptr);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************

#endif // !_KHEAP_H