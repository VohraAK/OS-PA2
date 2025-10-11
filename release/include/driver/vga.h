#ifndef _VGA_H
#define _VGA_H

#include <stdint.h>
#include <string.h>

#include "utils.h"

/**
 * Enumeration of VGA colors. Max value is 15, which corresponds to the color 
 * 	white. Can be used to set the foreground and background colors of text in 
 *  the VGA text mode.
 */

enum vga_color {
	VGA_COLOR_BLACK         = 0,
	VGA_COLOR_BLUE          = 1,
	VGA_COLOR_GREEN         = 2,
	VGA_COLOR_CYAN          = 3,
	VGA_COLOR_RED           = 4,
	VGA_COLOR_MAGENTA       = 5,
	VGA_COLOR_BROWN         = 6,
	VGA_COLOR_LIGHT_GREY    = 7,
	VGA_COLOR_DARK_GREY     = 8,
	VGA_COLOR_LIGHT_BLUE    = 9,
	VGA_COLOR_LIGHT_GREEN   = 10,
	VGA_COLOR_LIGHT_CYAN    = 11,
	VGA_COLOR_LIGHT_RED     = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN   = 14,
	VGA_COLOR_WHITE         = 15,
};

//! Defines for the VGA text mode
#define VGA_WIDTH  			80
#define VGA_HEIGHT 			25
#define VGA_ADDRESS 		0xB8000
#define VGA_SIZE 			(VGA_WIDTH * VGA_HEIGHT)
#define VGA_ENTRY_SIZE 		2 //bytes

//! Defines for the VGA hardware cursor
#define VGA_CRTC_INDEX_PORT 0x3D4
#define VGA_CRTC_DATA_PORT 	0x3D5

#define VGA_HARDWARE_CURSOR_CMD_SETHIGH 0x0E
#define VGA_HARDWARE_CURSOR_CMD_SETLOW  0x0F

//! typedef a VGA entry as a 16-bit unsigned integer
typedef uint16_t 	vga_entry_t;

// function prototypes
uint8_t vga_entry_color (enum vga_color fg, enum vga_color bg);
vga_entry_t vga_entry (unsigned char uc, uint8_t color);
void vga_move_cursor_to (uint8_t x, uint8_t y);
void vga_putentry_at (vga_entry_t entry, uint8_t x, uint8_t y);
vga_entry_t* vga_get_screen_buffer (void);

// helper functions
void vga_clear_buffer(uint8_t color);
void vga_settext_color(uint8_t color);
void vga_setbg_color(uint8_t color);
void vga_scrollup_one_line(uint8_t color);
void vga_removeentry_at(uint8_t x, uint8_t y);

#endif // _VGA_H