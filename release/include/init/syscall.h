#ifndef _SYSCALL_H
#define _SYSCALL_H
//*****************************************************************************
//*
//*  @file		syscall.h
//*  @author    Abdul Rafay (abdul.rafay@lums.edu.pk)
//*  @brief		System call interface header file
//*  @version	
//*
//****************************************************************************/
//-----------------------------------------------------------------------------
// 		REQUIRED HEADERS
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <interrupts.h>
#include <init/tty.h>

//-----------------------------------------------------------------------------
// 		INTERFACE DEFINES/TYPES
//-----------------------------------------------------------------------------

//! enum to define syscall numbers
typedef enum {

	SYSCALL_READ = 0,   //? syscall number for read: 0
	SYSCALL_WRITE = 1,      //? syscall number for write: 1
	SYSCALL_CLEAR = 2,

	/* REST will be added as the project progresses */
	
} syscall_nr;

//-----------------------------------------------------------------------------
// 		INTERFACE DATA STRUCTURES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// 		INTERFACE FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------

#define NUM_SYSCALLS 3
extern void* syscall_dispatch_table[NUM_SYSCALLS];

// defining typedefs for casting a void* into appropriate function pointer types
typedef void (*read_func_t)(char*, uint32_t);
typedef void (*write_func_t)(const char*, uint32_t);
typedef void (*generic_void_func_t)(void);


//! initialize the system's syscall interface
void syscall_init (void);

//*****************************************************************************
//**
//** 	END _[filename]
//**
//*****************************************************************************

void syscall_handler(interrupt_context_t *context);
void read(char* buffer, uint32_t size);
void write(const char* buffer, uint32_t size);
void clear(void);

#endif //! _SYSCALL_H