/*******************************************************************************
 * 
 * @file 	isr.h
 * @author  
 * @brief
 * 
 * The stubs for the first 32 CPU exceptions and interrupts + irqs are declared
 *  here.
 * 
 * These are the standard CPU exceptions and interrupts that are defined by the
 *  x86 architecture. These ISRs will be linked externally via the assembly
 *  implementations.
 * 
 * Note that interrupts 0-31 are reserved for CPU exceptions and interrupts, 
 *  and are not used for user-defined interrupts.
 *  Beyond that, you can define your own interrupts in the range 32-255.
 * 
 * **Exception classification:**
 * - Faults: These exceptions can be handled by the operating system, allowing
 *  		 it to recover. Ret address of the instruction that caused the 
 * 			 fault is pushed onto the stack, instead of the next one.
 * - Traps:  A trap is an exception that is reported immediately following
 * 			 the execution of the trapping instruction. Ret address of the next
 * 			 instruction is pushed.
 * - Aborts: An abort is an exception that does not always report the precise
 * 			 location of the instruction that caused the abort. It is used for
 * 			 unrecoverable errors, such as hardware failures. No return address
 * 			 is pushed onto the stack.
 * 
*******************************************************************************/

KERNEL_CODE_SEGMENT = 0x08      /* Kernel code segment offset in the GDT */
KERNEL_DATA_SEGMENT = 0x10      /* Kernel data segment offset in the GDT */

.extern    interrupt_dispatch   /* Declare the external C handler for ISRs */

/* Interrupt handlers entry points will look something like this:
    .globl isr0
isr0:
    pushl $0 # push dummy error code
    pushl $0 # push interrupt number
    jmp isr_common_handler # jump instead of a call


     */


/* We then call the macros to put in place handlers for all exceptions. Note
    that the definitions for all 256 entries will be provided on the per
    need basis. */

// -- ADD THE ASSEMBLY ENTRY POINTS HERE FOR THE 32 EXCEPTIONS -- //

// CPU exceptions

/*. Division Error */
.globl isr0
isr0:
    pushl $0    /* dummy error code */
    pushl $0
    jmp isr_common_handler

/*. Debug */
.globl isr1
isr1:
    pushl $0    /* dummy error code */
    pushl $1
    jmp isr_common_handler

/*. Non-maskable Interrupt */
.globl isr2
isr2:
    pushl $0    /* dummy error code */
    pushl $2
    jmp isr_common_handler

/*. Breakpoint */
.globl isr3
isr3:
    pushl $0    /* dummy error code */
    pushl $3
    jmp isr_common_handler

/*. Overflow */
.globl isr4
isr4:
    pushl $0    /* dummy error code */
    pushl $4
    jmp isr_common_handler

/*. Bound Range Exceeded */
.globl isr5
isr5:
    pushl $0    /* dummy error code */
    pushl $5
    jmp isr_common_handler

/*. Invalid Opcode */
.globl isr6
isr6:
    pushl $0    /* dummy error code */
    pushl $6
    jmp isr_common_handler

/*. Device Not Available */
.globl isr7
isr7:
    pushl $0    /* dummy error code */
    pushl $7
    jmp isr_common_handler

/*. Double Fault */
.globl isr8
isr8:
    pushl $0
    pushl $8
    jmp isr_common_handler

/*. Coprocessor Segment Overrun */
.globl isr9
isr9:
    pushl $0    /* dummy error code */
    pushl $9
    jmp isr_common_handler

/*. Invalid TSS */
.globl isr10
isr10:
    pushl $10
    jmp isr_common_handler

/* Segment Not Present */
.globl isr11
isr11:
    pushl $11
    jmp isr_common_handler

/* Stack-Segment Fault */
.globl isr12
isr12:
    pushl $12
    jmp isr_common_handler

/* Segment Not Present */
.globl isr13
isr13:
    pushl $13
    jmp isr_common_handler

/* Page Fault */
.globl isr14
isr14:
    pushl $14
    jmp isr_common_handler

/* Reserved */
.globl isr15
isr15:
    pushl $0    /* Dummy error code */
    pushl $15
    jmp isr_common_handler

/* x87 Floating-Point Exception */
.globl isr16
isr16:
    pushl $0    /* Dummy error code */
    pushl $16
    jmp isr_common_handler

/* Alignment Check */
.globl isr17
isr17:
    pushl $17
    jmp isr_common_handler

/* Machine Check */
.globl isr18
isr18:
    pushl $0    /* Dummy error code */
    pushl $18
    jmp isr_common_handler

/* SIMD Floating-Point Exception */
.globl isr19
isr19:
    pushl $0    /* Dummy error code */
    pushl $19
    jmp isr_common_handler

/* Virtualization Exception */
.globl isr20
isr20:
    pushl $0    /* Dummy error code */
    pushl $20
    jmp isr_common_handler

/* Control Protection Exception */
.globl isr21
isr21:
    pushl $21
    jmp isr_common_handler


/* Hypervisor Injection Exception */
.globl isr28
isr28:
    pushl $0    /* Dummy error code */
    pushl $28
    jmp isr_common_handler

/* VMM Communication Exception */
.globl isr29
isr29:
    pushl $29
    jmp isr_common_handler

/* Security Exception */
.globl isr30
isr30:
    pushl $30
    jmp isr_common_handler



// reserved interrupts from 22 to 31

/* Reserved Exceptions (22-27) and 31 */
.globl isr22
isr22:
    pushl $0    /* Dummy error code */
    pushl $22
    jmp isr_common_handler

.globl isr23
isr23:
    pushl $0    /* Dummy error code */
    pushl $23
    jmp isr_common_handler

.globl isr24
isr24:
    pushl $0    /* Dummy error code */
    pushl $24
    jmp isr_common_handler

.globl isr25
isr25:
    pushl $0    /* Dummy error code */
    pushl $25
    jmp isr_common_handler

.globl isr26
isr26:
    pushl $0    /* Dummy error code */
    pushl $26
    jmp isr_common_handler

.globl isr27
isr27:
    pushl $0    /* Dummy error code */
    pushl $27
    jmp isr_common_handler

.globl isr31
isr31:
    pushl $0    /* Dummy error code */
    pushl $31
    jmp isr_common_handler


/* Defining an ISR stub table for code brevity (inspired by https://wiki.osdev.org/Interrupts_Tutorial) */

// The following are the IRQ definitions for the PIC generated interrupts.
// master pic IRQs are from 0 to 7, and slave pic IRQs are from 8 to 15.

/* IRQs as ISRs 32 to 47 */
.globl isr32
isr32:
    pushl $0
    pushl $32
    jmp isr_common_handler

.globl isr33
isr33:
    pushl $0
    pushl $33
    jmp isr_common_handler

.globl isr34
isr34:
    pushl $0
    pushl $34
    jmp isr_common_handler

.globl isr35
isr35:
    pushl $0
    pushl $35
    jmp isr_common_handler

.globl isr36
isr36:
    pushl $0
    pushl $36
    jmp isr_common_handler

.globl isr37
isr37:
    pushl $0
    pushl $37
    jmp isr_common_handler

.globl isr38
isr38:
    pushl $0
    pushl $38
    jmp isr_common_handler

.globl isr39
isr39:
    pushl $0
    pushl $39
    jmp isr_common_handler

.globl isr40
isr40:
    pushl $0
    pushl $40
    jmp isr_common_handler

.globl isr41
isr41:
    pushl $0
    pushl $41
    jmp isr_common_handler

.globl isr42
isr42:
    pushl $0
    pushl $42
    jmp isr_common_handler

.globl isr43
isr43:
    pushl $0
    pushl $43
    jmp isr_common_handler

.globl isr44
isr44:
    pushl $0
    pushl $44
    jmp isr_common_handler

.globl isr45
isr45:
    pushl $0
    pushl $45
    jmp isr_common_handler

.globl isr46
isr46:
    pushl $0
    pushl $46
    jmp isr_common_handler

.globl isr47
isr47:
    pushl $0
    pushl $47
    jmp isr_common_handler

.globl isr_stubs
isr_stubs:
    .long isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15, isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31, isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39, isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47

// interrupt 0x80 is used for system calls, so we configure the entry for that.

/* Syscalls and other software defined interrupts */
.globl isr128
isr128:
    pushl $0
    pushl $128
    jmp isr_common_handler

/**
 * @brief Common ISR handler that saves the state, calls the C handler, 
 *          and restores the state. This function is called by all ISRs to
 *          handle the interrupt. It saves the registers, sets up the segment
 *          registers, calls the C handler (interrupt_dispatch), and restores
 *          the state before returning.
 */

.type isr_common_handler, @function
isr_common_handler:
    
    /* 1.save1 the GP registers */
    /* in the order: eax, ecx, edx, ebx, old esp, ebp, esi, edi*/
    pushl %eax
    pushl %ecx
    pushl %edx
    pushl %ebx

    /* since I have pushed 4 registers, the stack pointer has moved down 16 bytes */
    lea 16(%esp), %ebx
    pushl %ebx

    pushl %ebp
    pushl %esi
    pushl %edi
    
    /* 2. save the data segment register */
    pushl %ds

    /* 3. Call the C handler for the interrupt dispatching */
    pushl %esp   
    call interrupt_dispatch


    /* clean up the stack and restore the state */

    /* skip over pushed address (checked from LLM) */
    addl $4, %esp

    /* restore the gp registers */

    /* stack cleanup, must be restored to the state before the ISR. Done because
        the iret instruction expects the previously automatically pushed fields:
        ss, eflags, cs, ip. These are pushed automatically by the CPU when an
        interrupt occurs. */
    
    popl %ds

    popl %edi
    popl %esi
    popl %ebp

    /* skip over the original esp location (from LLM) */
    addl $4, %esp

    popl %ebx
    popl %edx
    popl %ecx
    popl %eax

    /* skip over the error code and interrupt number for iret */
    addl $8, %esp

    /* return from the interrupt, restoring the flags and segments */
    iret
