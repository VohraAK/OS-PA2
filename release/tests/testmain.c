#include <driver/serial.h>

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <testmain.h>
#include <init/tests.h>
#include <mm/tests.h>


/* All commands received from the orchestrator are read in this buffer */

#define CMD_BUF_SIZE	128
static  char 	cmd_buf [CMD_BUF_SIZE];

/* List of all the test name to test function mappings. All commands are sent 
	as a single word, and the corresponding test is ran in the kernel. Support
	for params can be added later by using simple strtok calls. */

static struct test_case test_cases[] = {

	// { "vga_entry", 				test_vga_entry },
	// { "vga_cursor", 			test_vga_cursor },
	// { "vga_entry_overwrite", 	test_vga_entry_overwrite },
	// { "vga_color", 				test_vga_entry_colors },
	// { "vga_entry_boundaries", 	test_vga_entry_boundaries },

	// { "intr_reg", 				test_intr_reg },
	// { "intr_unreg", 			test_intr_unreg },
	// { "intr_unreg", 			test_intr_unreg },
	// { "intr_multi", 			test_intr_multi },

	// { "kbd_basic", 				test_kbd_basic },
	// { "kbd_multi", 				test_kbd_multi },
	// { "kbd_capslock", 			test_kbd_capslock },
	// { "kbd_shift", 				test_kbd_shift },

	// // ---- TTY tests ----
	// { "terminal_getc",          test_terminal_getc },
	// { "terminal_read",          test_terminal_read },
	// { "terminal_cursor",        test_terminal_cursor },
	// { "terminal_clear",         test_terminal_clear },
	// { "terminal_putc",          test_terminal_putc },
	// { "terminal_write",         test_terminal_write },
	// { "terminal_column",        test_terminal_column },

	// // ---- SYSCALL TESTS ----
	// { "syscall_register",       test_syscall_register },
	// { "syscall_read",       	test_syscall_read },
	// { "syscall_write",       	test_syscall_write },

	// // ---- SHELL
	// { "shell_echo",				test_shell_echo },
	// { "shell_repeat",			test_shell_repeat_n },
	// { "shell_clear",			test_shell_clear },
	// { "shell_colour",			test_shell_text_colour },
	// { "shell_bgcolour",			test_shell_bg_colour },

    // // ---- KHEAP tests ----
    // { "kheap_init",           	test_kheap_init },
    // { "kheap_alloc_small",    	test_kheap_alloc_small },
    // { "kheap_alloc_exact",    	test_kheap_alloc_exact },
    // { "kheap_split",          	test_kheap_split },
    // { "kheap_free_reuse",     	test_kheap_free_reuse },
    // { "kheap_coalesce",       	test_kheap_coalesce },
    // { "kheap_double_free",    	test_kheap_double_free },
    // { "kheap_invalid_free",   	test_kheap_invalid_free },
    // { "kheap_realloc_shrink", 	test_kheap_realloc_shrink },
    // { "kheap_realloc_expand", 	test_kheap_realloc_expand },
    // { "kheap_realloc_null",   	test_kheap_realloc_null },
    // { "kheap_realloc_zero",   	test_kheap_realloc_zero },
    // { "kheap_oom",            	test_kheap_oom },
    // { "kheap_stress_pattern", 	test_kheap_stress_pattern },

    // // ---- KMM tests ----
    { "kmm_init_total",       	test_kmm_init_total },
    { "kmm_reserved",         	test_kmm_reserved_regions },
    { "kmm_alloc_all",        	test_kmm_alloc_all },
    { "kmm_alloc_align",      	test_kmm_alloc_alignment },
    { "kmm_reuse",            	test_kmm_reuse_freed },
    { "kmm_double_free",      	test_kmm_double_free },
    { "kmm_free_invalid",     	test_kmm_free_invalid },
    { "kmm_consistency",      	test_kmm_consistency },
    { "kmm_pattern",          	test_kmm_pattern_alloc_free },
    { "kmm_oom",              	test_kmm_oom },

    // // ---- VMM tests ----
	// { "vmm_init",             	test_vmm_init },
    // { "vmm_get_kerneldir",    	test_vmm_get_kerneldir },
    // { "vmm_get_currentdir",   	test_vmm_get_current_pagedir },
    // { "vmm_create_space",     	test_vmm_create_address_space },
    // { "vmm_switch_dir",       	test_vmm_switch_pagedir },
    // { "vmm_create_pt",        	test_vmm_create_pt },
    // { "vmm_map_basic",        	test_vmm_map_page_basic },
    // { "vmm_page_alloc",       	test_vmm_page_alloc },
    // { "vmm_page_free",        	test_vmm_page_free },
	// { "vmm_alloc_region",		test_vmm_alloc_region},
	// { "vmm_free_region",		test_vmm_free_region},
    // { "vmm_get_phys",         	test_vmm_get_phys_frame },
    // { "vmm_double_map",       	test_vmm_double_mapping },
	// { "vmm_clone_pagetable",	test_vmm_clone_pagetable},
    // { "vmm_clone_dir",        	test_vmm_clone_pagedir },

	{ NULL, NULL } // marks the end of the array

};

/* Responsible for test orchestration inside the kernel. */

void start_tests () {

	serial_init (false); // initialize the serial port in polling mode

	/* testing cycle is simple, read command, dispatch to function, repeat */
	while (1) {

		size_t cmd_size = read_command ();
		if (cmd_size == 0) {
			continue; // empty command, ignore
		}

		bool found = false;

		for (size_t i = 0; test_cases[i].name != NULL; i++) {

			if (strcmp (cmd_buf, test_cases[i].name) == 0) {

				found = true;
				test_cases[i].func (); // call the test function
				break;

			}

		}

		if (!found) {
			send_msg ("Unknown command");
		}

	}

}

/* Reads a command from the serial port into cmd_buf and returns
   the number of characters read. The command is terminated by a any of these:
   '\0', '\n', '\r'. */

size_t read_command () {

	size_t start = 0;
	memset (cmd_buf, 0, CMD_BUF_SIZE);

	while (1) {
		char c = serial_getc ();

		if (c == '\0' || c == '\n' || c == '\r') {

			cmd_buf [start] = '\0';
			break;
		
		} else {

			if (start < CMD_BUF_SIZE - 1) {
				cmd_buf [start++] = c;
			}
		
		}	
	}

	return start;
}

/* Send a message string back to the server. A "*" is used to mark the end
	of the message. */

void send_msg (const char *msg) {
	serial_puts (msg);
	serial_putc ('*'); // end of message marker
}