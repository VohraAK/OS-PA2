#ifndef _VGA_C
#define _VGA_C

#include "driver/vga.h"


static uint8_t terminal_cursor_x;
static uint8_t terminal_cursor_y;
static uint8_t terminal_color;

uint8_t vga_entry_color (enum vga_color fg, enum vga_color bg)
{
    // TODO: check range of colors

    // I need to set the high and low nibbles of the 8-bit register

    uint8_t color = (uint8_t) 0;
    uint8_t high_nibble = (uint8_t) bg;
    uint8_t low_nibble = (uint8_t) fg;
    
    // since the high nibble is right now a byte (e.g. 0x0A), I need to
    // shift by 4 bits for the high position
    
    // to set/change a bit, I need to AND it
    // to copy a bit, I need to OR it ✔

    color = (high_nibble << 4) | (low_nibble);

    return color;

}

vga_entry_t vga_entry (unsigned char uc, uint8_t color)
{
    // byte 0 will contain the ASCII value of the character
    // byte 1 wll contain the color

    vga_entry_t entry = (vga_entry_t) 0;

    // same strategy; shift color to left by 8 bits to make room for char
    entry = (color << 8) | (uc);

    return entry;

}

void vga_move_cursor_to (uint8_t x, uint8_t y)
{
    // TODO: check range of x and y
    if (y >= VGA_HEIGHT || x >= VGA_WIDTH)
        return;

    // VGA text mode screen: 80x25
    // need to convert coordinates to a linear position
    // example (x, y) = (3, 4) --> 2nd row, 3rd column 

    uint16_t linear_pos = (y)*VGA_WIDTH + (x);

    // I now need to write the linear position to the CRTC
    // need to set high and low bytes of CRTC using specific ports

    // I can right bitshift by 8 to get the high byte
    uint8_t high = (linear_pos >> 8);

    // I can extract the low byte by ANDing with 0xFF -> zero-extension ftw
    uint8_t low = (linear_pos & 0xFF);

    outb (VGA_HARDWARE_CURSOR_CMD_SETHIGH, VGA_CRTC_INDEX_PORT);
    outb (high, VGA_CRTC_DATA_PORT);

    outb (VGA_HARDWARE_CURSOR_CMD_SETLOW, VGA_CRTC_INDEX_PORT);
    outb (low, VGA_CRTC_DATA_PORT);

}

void vga_putentry_at (vga_entry_t entry, uint8_t x, uint8_t y)
{
    // TODO: check range of x and y
    if (y >= VGA_HEIGHT || x >= VGA_WIDTH)
        return;

    // I should get the address of the video memory using the helper function
    vga_entry_t* v_mem = vga_get_screen_buffer();

    // Calculate linear position as before
    uint16_t linear_pos = (y)*VGA_WIDTH + (x);

    // This pointer can help me place vga_entry_t chunks of data into memory
    // I can just use memory indexing
    v_mem[linear_pos] = entry;

}

vga_entry_t* vga_get_screen_buffer (void)
{
    // I should return a pointer to the VGA address
    return (vga_entry_t*) VGA_ADDRESS;
}

// helper functions
void vga_clear_buffer(uint8_t color)
{
    vga_entry_t* v_mem = vga_get_screen_buffer();

    // replace with spaces, but leave the color
    for (uint8_t i = 0; i < VGA_HEIGHT; i++)
    {
        for (uint8_t j = 0; j < VGA_WIDTH; j++)
        {
            // straight from test cases
            v_mem[i * VGA_WIDTH + j] = vga_entry(' ', color);
        }
    }

}

void vga_settext_color(uint8_t color)
{
    // set fg color for all entries in buffer

    vga_entry_t* v_mem = vga_get_screen_buffer();

    for (uint8_t i = 0; i < VGA_HEIGHT; i++)
    {
        for (uint8_t j = 0; j < VGA_WIDTH; j++)
        {
            // set fg (byte 1's low 4 bits)
            vga_entry_t entry = v_mem[i * VGA_WIDTH + j];
            entry = (entry & 0xF0FF) | (color << 8);

            v_mem[i * VGA_WIDTH + j] = entry;
        }
    }
}

void vga_setbg_color(uint8_t color)
{
    // set fg color for all entries in buffer

    vga_entry_t* v_mem = vga_get_screen_buffer();

    for (uint8_t i = 0; i < VGA_HEIGHT; i++)
    {
        for (uint8_t j = 0; j < VGA_WIDTH; j++)
        {
            // set bg (byte 1's high 4 bits)
            vga_entry_t entry = v_mem[i * VGA_WIDTH + j];   // i*cols +j formula ftw
            entry = (entry & 0x0FFF) | (color << 12);

            v_mem[i * VGA_WIDTH + j] = entry;

        }
    }
}

void vga_scrollup_one_line(uint8_t color)
{
    vga_entry_t* v_mem = vga_get_screen_buffer();

    // clear the first line
    for (uint8_t i = 0; i < VGA_WIDTH; i++)
    {
        v_mem[i] = 0;
    }

    // shift all entries up by one line
    for (uint8_t i = 1; i < VGA_HEIGHT; i++) 
    {
        for (uint8_t j = 0; j < VGA_WIDTH; j++) 
        {
            v_mem[(i - 1) * VGA_WIDTH + j] = v_mem[i * VGA_WIDTH + j];
        }
    }
    
    // clear the last line
    for (uint8_t j = 0; j < VGA_WIDTH; j++) 
    {
        v_mem[(VGA_HEIGHT - 1) * VGA_WIDTH + j] = vga_entry(' ', color);
    }
}

void vga_removeentry_at(uint8_t x, uint8_t y)
{
    vga_entry_t* v_mem = vga_get_screen_buffer();

    // index at position
    uint16_t linear_pos = (y)*VGA_WIDTH + (x);

    // replace entry with backspace, leaving behind the color
    vga_entry_t to_be_removed = v_mem[linear_pos];

    uint8_t color = (to_be_removed >> 8);

    v_mem[linear_pos] = vga_entry(' ', color);

}

#endif