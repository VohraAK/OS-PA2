#ifndef _IDT_H
#define _IDT_H
//******************************************************************************
//*
//*  @file		idt.h
//*  @author
//*  @brief		Contains the definitions and functions to setup the Interrupt 
//*             Descriptor Table (IDT) for the x86 architecture. The IDT is 
//*             used by the CPU to determine how to handle interrupts and 
//*             exceptions.
//*  @version	0.1
//*
//*****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "init/isr.h"

//-----------------------------------------------------------------------------
// 		PUBLIC INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

/***  Macros for IDT Gate types  ***/

//! Task gate (offset unused)
#define IDT_GATE_TYPE_TASK       0x5

//! 16-bit interrupt gate
#define IDT_GATE_TYPE_16_INT     0x6

//! 16-bit trap gate
#define IDT_GATE_TYPE_16_TRAP    0x7

//! 32-bit interrupt gate
#define IDT_GATE_TYPE_32_INT     0xE

//! 32-bit trap gate
#define IDT_GATE_TYPE_32_TRAP    0xF

/***  Macros for IDT Gate attributes  ***/

//! Gate is present (annd valid)
#define IDT_ATTR_PRESENT        0x80

//! For memory protection bits
#define IDT_ATTR_DPL0           0x00  //! DPL 0 (highest privilege)
#define IDT_ATTR_DPL1           0x20  //! DPL 1
#define IDT_ATTR_DPL2           0x40  //! DPL 2
#define IDT_ATTR_DPL3           0x60  //! DPL 3 (lowest privilege)
  

//-----------------------------------------------------------------------------
// 		PUBLIC INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------

/*******************************************************************************
 * 
 *  @struct  idt_entry_t
 *  @brief   Represents a single entry in the Interrupt Descriptor Table (IDT). 
 *           Each entry corresponds to an interrupt or exception handler, and 
 *           is called a "gate".
 *	
******************************************************************************/
typedef struct {

    /* Add the IDT entry fields here */

    // ORDER MATTERS HERE!
    
    // bytes 0-4
    uint16_t offset_low_16;
    uint16_t selector;
    uint8_t reserved;

    // byte 5 (gate_type_attr)
    uint8_t gate_type_attr;

    uint16_t offset_high_16;


} __attribute__((packed)) idt_entry_t;


/*******************************************************************************
 * 
 *   @struct    idt_ptr_t
 *   @brief     Represents the 48-bit pointer to the IDT in memory. This 
 *              structure is used to load the IDT into the CPU's IDTR register.
 *   @note      The IDT pointer structure contains the size of the IDT in bytes
 * 
 ******************************************************************************/
typedef struct {

    //! Size of the IDT in bytes
    uint16_t            limit;

    //! Base address of the IDT
    uint32_t            base;

} __attribute__((packed)) idt_ptr_t;

//-----------------------------------------------------------------------------
// 		INTERFACE PROTOTYPES
//-----------------------------------------------------------------------------

/*******************************************************************************
 * 
 *   @function  load_init
 *   @brief     Loads the IDT into the CPU's IDTR register. Expects a pointer to
 *              an idt_ptr_t structure that contains the base address and limit
 *              of the IDT. This gets loaded in the IDTR register using the lidt
 *              instruction.
 * 
*******************************************************************************/
void load_idt (uint32_t);

// functions to be implemented
void create_idt_entry(idt_entry_t *entry, uint32_t offset, uint16_t selector, uint8_t gate_type_attr);

void idt_init();

// self-defined macros
#define SEGMENT_SELECTOR 0x08
#define IDT_ENTRY_SIZE sizeof(idt_entry_t) 
#define IDT_TABLE_SIZE 256      
#define IDT_32BIT_EXCEPTION_GATE (IDT_ATTR_PRESENT | IDT_ATTR_DPL0 | IDT_GATE_TYPE_32_INT)   // 0b10001110 -> 0x8E 
#define IDT_IRQ_GATE (IDT_ATTR_PRESENT | IDT_ATTR_DPL0 | IDT_GATE_TYPE_32_INT)   // 0b10001110 -> 0x8E 
#define IDT_SYSCALL_GATE (IDT_ATTR_PRESENT | IDT_ATTR_DPL3 | IDT_GATE_TYPE_32_TRAP) // lowest privledge


// self-defined tings
extern idt_entry_t idt_entries[IDT_TABLE_SIZE];
extern idt_ptr_t idt_ptr;
void delete_idt_entry(idt_entry_t *entry);

#endif // _IDT_H