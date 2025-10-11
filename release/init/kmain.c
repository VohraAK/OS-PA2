#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <init/gdt.h>
#include <interrupts.h>
#include <driver/keyboard.h>
#include <init/tty.h>
#include <init/syscall.h>
#include <init/shell.h>

#ifdef TESTING
extern void start_tests ();
#endif

//! this is currently a kernel commandline, built for debugging kernel
//! add debug commands as needed to the shell interpreter
extern void shell ();
static void hlt ();			//! halts the CPU and disables interrupts

//! helper to zero out the BSS section
static void zero_bss ();

void kmain () 
{

	zero_bss ();					// Zero out the BSS section
	gdt_init_flat_protected (); 	// initialize the system segments

	/* Your implementation starts here */

	setup_x86_interrupts();
	kbd_init();
	tty_init();
	syscall_init();

	/* Your implementation ends here */

#ifdef TESTING
	start_tests (); 				// Run kernel tests (dont modify)
#endif
	shell();
	hlt ();

}

static void zero_bss () {

	extern char kbss_start, kbss_end; // from linker script
	memset (&kbss_start, 0, &kbss_end - &kbss_start);

}

void hlt () {
	
	asm volatile ("cli; hlt");

}