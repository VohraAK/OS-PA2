#ifndef TESTMAIN_H
#define TESTMAIN_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

/* represents a single test case */
struct test_case {

	const char *name;
	void (*func) ();	/* note the absence of params, registered functions
							   may or may not take any param */
};

/* Reads a command from the serial port into cmd_buf and returns
   the number of characters read. The command is terminated by a any of these:
   '\0', '\n', '\r'. */

size_t 	read_command ();

/* Send a message string back to the server. A "*" is used to mark the end
	of the message. */

void 	send_msg (const char *msg);


/* Useful macros. */

#define PASS() send_msg ("PASSED")
#define FAIL() send_msg ("FAILED")

#endif // TESTMAIN_H
