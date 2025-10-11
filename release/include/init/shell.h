#ifndef _SHELL_H
#define _SHELL_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <init/tty.h>
#include <init/syscall.h>
#include <stdio.h>
#include <string.h>

#define NUM_CMD 7
#define MAX_TOKENS 16
#define BUFFER_SIZE 1024

// dispatch table struct
typedef struct 
{
    const char* name;
    void* handler;
    const char* description;
} dispatch_entry_t;

// colors

    #define BLACK         0x0
    #define BLUE          0x1
    #define GREEN         0x2
    #define CYAN          0x3
    #define RED           0x4
    #define MAGENTA       0x5
    #define BROWN         0x6
    #define LIGHTGREY     0x7
    #define DARKGREY      0x8
    #define LIGHTBLUE     0x9
    #define LIGHTGREEN    0xA
    #define LIGHTCYAN     0xB
    #define LIGHTRED      0xC
    #define LIGHTMAGENTA  0xD
    #define LIGHTBROWN    0xE
    #define WHITE         0xF

// command dispatch table
extern dispatch_entry_t cmd_dispatch_table[NUM_CMD];

// API declarations
void shell(void);
void run_cmd(char* cmd);

// helpers
void help_cmd(char* args);
void echo_cmd(char* args);
void clear_cmd(char* args);
void color_cmd(char* args);
void bg_color_cmd(char* args);
void repeat_text_cmd(char* args);
void exit_cmd(char* args);


#endif