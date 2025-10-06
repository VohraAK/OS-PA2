#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <driver/keyboard.h>
#include <testmain.h>
#include <init/tests.h>
#include <driver/vga.h>
#include <utils.h>
#include <init/tty.h>
#include <driver/serial.h>


void test_terminal_getc(){
    char c = terminal_getc();
    serial_putc(c);
    serial_putc('*');
}

void test_terminal_read(){
    char buffer[100];
    terminal_read(buffer, sizeof(buffer));
    send_msg(buffer);
}

void test_terminal_cursor(){
    int col = 5;
    int row = 2;
    terminal_move_cursor(col, row);
    uint16_t pos = 0;

    vga_move_cursor_to (col,row);

    // Select high byte register (0x0E), then read it
    outb(0x0E, VGA_CRTC_INDEX_PORT);
    pos = ((uint16_t) inb(VGA_CRTC_DATA_PORT)) << 8;

    // Select low byte register (0x0F), then read it
    outb(0x0F, VGA_CRTC_INDEX_PORT);
    pos |= inb(VGA_CRTC_DATA_PORT);
    int row_new = pos / VGA_WIDTH;
    int col_new = pos % VGA_WIDTH;
    if(row == row_new && col == col_new){
        send_msg ("PASSED");
        return;
    }
    
    send_msg ("FAILED: failed to move cursor to expected position");
}

void test_terminal_clear(){
    terminal_clear_scr();
    vga_entry_t* screen = vga_get_screen_buffer();
    uint8_t colour = terminal_get_colour();
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        if (screen[i] != vga_entry(' ', colour)) {
            send_msg("FAILED");
            return;
        }
    }
    uint16_t pos = 0;

    // Select high byte register (0x0E), then read it
    outb(0x0E, VGA_CRTC_INDEX_PORT);
    pos = ((uint16_t) inb(VGA_CRTC_DATA_PORT)) << 8;

    // Select low byte register (0x0F), then read it
    outb(0x0F, VGA_CRTC_INDEX_PORT);
    pos |= inb(VGA_CRTC_DATA_PORT);
    int row_new = pos / VGA_WIDTH;
    int col_new = pos % VGA_WIDTH;
    if(0 == row_new && 0 == col_new){
        send_msg ("PASSED");
        return;
    }
    send_msg("FAILED");
}

void test_terminal_putc(){
    char c = 'a';
    int row = 5;
    int col = 3;
    terminal_move_cursor(col,row);
    terminal_putc(c);
    
    vga_entry_t* screen = vga_get_screen_buffer();
    int offset = row * VGA_WIDTH + col;
    uint8_t colour = terminal_get_colour();
    if (screen[offset] == vga_entry(c,colour)) {
        send_msg("PASSED");
        return;
    }
    send_msg("FAILED");
}

void test_terminal_write(){
    char* str = "hello world";
    int row = 4;
    int col = 4;
    terminal_move_cursor(row,col);
    terminal_write(str, strlen(str));
    vga_entry_t* screen = vga_get_screen_buffer();
    int len = strlen(str);
    int passed = 1;
    int offset = row * VGA_WIDTH + col; 
    uint8_t colour = terminal_get_colour();
    for (int i = 0; i < len; i++) {
        if (screen[i+offset] != vga_entry(str[i], colour)) {
            passed = 0;
            break;
        }
    }
    if (passed) {
        send_msg("PASSED");
        return;
    }
    send_msg("FAILED");
}

void test_terminal_column(){
    char c= 'd';
    int row = 10;
    terminal_move_cursor(VGA_WIDTH-1,row);
    terminal_putc(c);
    uint16_t pos = 0;

    // Select high byte register (0x0E), then read it
    outb(0x0E, VGA_CRTC_INDEX_PORT);
    pos = ((uint16_t)inb(VGA_CRTC_DATA_PORT)) << 8;

    // Select low byte register (0x0F), then read it
    outb(0x0F, VGA_CRTC_INDEX_PORT);
    pos |= inb(VGA_CRTC_DATA_PORT);
    int row_new = pos / VGA_WIDTH;
    int col_new = pos % VGA_WIDTH;
    if(row+1 == row_new && 0 == col_new){
       send_msg("PASSED");
       return;
    }
    send_msg("FAILED");    
}