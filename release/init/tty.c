#ifndef _TTY_C
#define _TTY_C

#include <init/tty.h>

// tty state
uint8_t terminal_cursor_x = 0;
uint8_t terminal_cursor_y = 0;
uint8_t terminal_color = 0;


void tty_init(void)
{
    // initialise stdout
    terminal_stdout_init();

    // initialise stdin
    terminal_stdin_init();

}

void terminal_stdout_init(void)
{
    // initialise state
    terminal_cursor_x = 0;
    terminal_cursor_y = 0;

    // reset terminal color
    terminal_reset_color();

    // clear screen
    terminal_clear_scr();
}

void terminal_stdin_init(void)
{
    // TODO?
}

void terminal_clear_scr(void)
{
    // clear the VGA buffer
    vga_clear_buffer(terminal_color);

    // reset cursor
    terminal_cursor_x = 0;
    terminal_cursor_y = 0;

    vga_move_cursor_to(terminal_cursor_x, terminal_cursor_y);

}

void terminal_move_cursor(uint8_t x, uint8_t y)
{
    // move cursor in vga
    vga_move_cursor_to(x, y);

    // update state
    terminal_cursor_x = x;
    terminal_cursor_y = y;
}

void terminal_setcolor(uint8_t color)
{
    // update state
    terminal_color = color;
}

void terminal_settext_color (uint8_t color)
{
    // apply this color to the entire vga buffer entries
    vga_settext_color(color);

    // update terminal color state (FG)
    terminal_color = (terminal_color & 0xF0) | (color);
}

void terminal_setbg_color (uint8_t color)
{
    // apply this color to the entire vga buffer entries
    vga_setbg_color(color);

    // update terminal color state
    terminal_color = (terminal_color & 0x0F) | (color << 4);
}

char terminal_getc (void)
{
    // block until a character is available from the keyboard buffer
    KBD_ENTRY entry = kbd_getlastkey_buf();

    return entry.ascii;
}

void terminal_reset_color (void)
{
    terminal_color = TTY_DEFAULT_COLOR;
}

void terminal_putc (char c)
{
    // check for newline
    if (c == '\n' || c == '\r')
    {
        // advance to new line, checking if scroll up is needed
        terminal_cursor_x = 0;
        terminal_cursor_y++;

    }

    // improved backspace handling (robust)
    else if (c == '\b')
    {
        // nothing to delete at origin
        if (terminal_cursor_x != 0 || terminal_cursor_y != 0)
        {
            // if at column 0 but not first line, go to previous line "after" last column
            if (terminal_cursor_x == 0)
            {
                terminal_cursor_y--;
                terminal_cursor_x = VGA_WIDTH;
            }

            // move one position left and clear that cell
            terminal_cursor_x--;
            vga_removeentry_at(terminal_cursor_x, terminal_cursor_y);
        }
    }

    else
    {
        // make vga_entry for c
        vga_entry_t entry = vga_entry(c, terminal_color);
        
        // write c to the buffer
        vga_putentry_at(entry, terminal_cursor_x, terminal_cursor_y);
    
        // update cursor offset
        terminal_cursor_x++;
    }

    // now check for line wrapping and screen scrolling

    // 1) line wrapping needed when cursor_x reaches WIDTH
    if (terminal_cursor_x >= VGA_WIDTH)
    {
        // update cursor offset
        terminal_cursor_x = 0;

        terminal_cursor_y++;
    }

    // check if cursor_y is past its limit
    if (terminal_cursor_y >= VGA_HEIGHT)
    {
        // scroll up!
        terminal_scrollup_one_line(); 

        // cursor_y is at the last line
        terminal_cursor_y = VGA_HEIGHT - 1;
    }

    // update cursor position
    vga_move_cursor_to(terminal_cursor_x, terminal_cursor_y);

}


void terminal_write(const char* data, uint32_t size)
{
    // call terminal_putc for every character in data
    for (uint32_t i = 0; i < size; i++)
        terminal_putc(data[i]);

}

void terminal_writestring (const char* data)
{
    // get the size of the data string using strlen
    uint32_t size = strlen(data);

    terminal_write(data, size);
}

void terminal_read (char* buffer, uint32_t size)
{
    // function reads the input from KBD buffer into this buffer AND echo!

    // need to handle backspace and return
    // backspace -> delete last added character and update cursor
    // return -> terminate input

    if (size == 0)
        return;

    // loop until return
    uint32_t buf_count = 0;

    while (buf_count < size - 1)
    {
        char c = terminal_getc();
        
        // check if return
        if (c == '\n' || c == '\r')
        {
            terminal_putc('\n');

            break;
        }

        // check if backspace
        else if (c == '\b')
        {
            
            if (buf_count > 0)
            {
                buf_count--;
                terminal_putc(c);
            }

        }

        else
        {
            // add to buffer if count does not exceed size
            buffer[buf_count++] = c;
            terminal_putc(c);
        }

    }

    buffer[buf_count] = '\0';

}

uint8_t terminal_get_colour (void)
{
    return terminal_color;
}

void terminal_scrollup_one_line(void)
{
    // shift all vga_entries up one line
    vga_scrollup_one_line(terminal_color);

    // set cursor_y to last line
    terminal_cursor_y = VGA_HEIGHT - 1;

    // set cursor_x to 0
    terminal_cursor_x = 0;
}


#endif