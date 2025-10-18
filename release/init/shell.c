#ifndef _SHELL_C
#define _SHELL_C

#include <init/shell.h>

// shell running status
static bool shell_active = true;

// define a command dispatch table
dispatch_entry_t cmd_dispatch_table[NUM_CMD] =
{
    {"help", help_cmd, "help | Display available shell commands and their descriptions.\n"},
    {"echo", echo_cmd, "echo [text] | Display the provided text arguments.\n"},
    {"clear", clear_cmd, "clear | Clears the entire screen and resets cursor to top-left.\n"},
    {"color", color_cmd, "(Add available text colors!) color [name] | Change text color.\n"},
    {"bgcolor", bg_color_cmd, "(Add available colors!) bgcolor [name] | Changes background color.\n"},
    {"repeat", repeat_text_cmd, "repeat [n] [text] | Display text n times.\n"},
    {"exit", exit_cmd, "exit | Exit shell.\n"}
};

void shell(void)
{
    // just to be safe
    // clear();

    while (shell_active)
    {
        if (shell_active == false)
            return;

        printf("noobsh> ");
        
        // get line
        char buf[BUFFER_SIZE];

        if (getline(buf, BUFFER_SIZE) < 0)
            continue;
        
        // strip trailing newline/carriage return
        uint32_t len = strlen(buf);

        if (len == 0)
        return;
        
        // check if buffer is empty
        if (buf[0] == '\0') 
            continue;

        // parse the commands
        run_cmd(buf);
    }
}

void run_cmd(char* buf)
{
    // some tokenisation done using LLMs

    uint32_t i = 0;
    
    // skip any trailing whitespaces from buffer
    while (buf[i] == ' ' || buf[i] == '\t')
        i++;
    
    // check if buffer is empty or has trailing whitespaces
    if (buf[i] == '\0')
        return;

    // extract first token (command name)
    char* cmd = &buf[i];
    
    // find end of first token
    while (buf[i] && buf[i] != ' ' && buf[i] != '\t')
        i++;
    
    // save the position where arguments start
    char* args_start;

    if (buf[i] != '\0')
    {
        buf[i] = '\0';  // null-terminate command
        i++;
        
        // skip whitespace before arguments
        while (buf[i] == ' ' || buf[i] == '\t')
            i++;
        
        // set args_start to beginning of arguments (or '\0' if no args)
        args_start = &buf[i];
    }

    else
    {
        args_start = &buf[i];
    }

    // check command against dispatch table

    for (uint32_t j = 0; j < NUM_CMD; j++)
    {
        if (strcmp(cmd, cmd_dispatch_table[j].name) == 0)
        {
            void (*handler)(char*) = cmd_dispatch_table[j].handler;
            handler(args_start);
            return;
        }
    }
    printf("Unknown command: %s\n", cmd);
    
}


// command handlers
void help_cmd(char* args)
{
    printf("\nAvailable commands:\n");
    for (uint32_t i = 0; i < NUM_CMD; i++)
    {
        printf("%s", cmd_dispatch_table[i].description);
    }
}

void echo_cmd(char* args)
{
    write(args, strlen(args));
    printf("\n");
}

void clear_cmd(char* args)
{
    clear();
}

void color_cmd(char* args)
{
    // skip any whitespace
    while (*args == ' ' || *args == '\t')
        args++;

    if (*args == '\0')
    {
        printf("Usage: color [name]\n");
        printf("Available: black blue green cyan red magenta brown light_grey dark_grey light_blue light_green light_cyan light_red light_magenta light_brown white\n");

        return;
    }

    // extract color name from args
    char* color_name = args;
    char* end = args;

    while (*end && *end != ' ' && *end != '\t')
        end++;

    // null-terminate the color name
    if (*end)
        *end = '\0';

    // check colors
    if (strcmp(color_name, "black") == 0)            
        terminal_settext_color(BLACK);
    else if (strcmp(color_name, "blue") == 0)        
        terminal_settext_color(BLUE);
    else if (strcmp(color_name, "green") == 0)       
        terminal_settext_color(GREEN);
    else if (strcmp(color_name, "cyan") == 0)        
        terminal_settext_color(CYAN);
    else if (strcmp(color_name, "red") == 0)         
        terminal_settext_color(RED);
    else if (strcmp(color_name, "magenta") == 0)     
        terminal_settext_color(MAGENTA);
    else if (strcmp(color_name, "brown") == 0)       
        terminal_settext_color(BROWN);
    else if (strcmp(color_name, "light_grey") == 0)  
        terminal_settext_color(LIGHTGREY);
    else if (strcmp(color_name, "dark_grey") == 0)   
        terminal_settext_color(DARKGREY);
    else if (strcmp(color_name, "light_blue") == 0)  
        terminal_settext_color(LIGHTBLUE);
    else if (strcmp(color_name, "light_green") == 0) 
        terminal_settext_color(LIGHTGREEN);
    else if (strcmp(color_name, "light_cyan") == 0)  
        terminal_settext_color(LIGHTCYAN);
    else if (strcmp(color_name, "light_red") == 0)   
        terminal_settext_color(LIGHTRED);
    else if (strcmp(color_name, "light_magenta") == 0) 
        terminal_settext_color(LIGHTMAGENTA);
    else if (strcmp(color_name, "light_brown") == 0) 
        terminal_settext_color(LIGHTBROWN);
    else if (strcmp(color_name, "white") == 0)       
        terminal_settext_color(WHITE);
    
    else 
    {
        printf("Unknown color: %s\n", color_name);
        printf("Available: black blue green cyan red magenta brown light_grey dark_grey light_blue light_green light_cyan light_red light_magenta light_brown white\n");
        return;
    }

}

void bg_color_cmd(char* args)
{
    // similar to text color

    // skip any whitespace
    while (*args == ' ' || *args == '\t')
        args++;

    if (*args == '\0')
    {
        printf("Usage: bgcolor [name]\n");
        printf("Available: black blue green cyan red magenta brown light_grey dark_grey light_blue light_green light_cyan light_red light_magenta light_brown white\n");
        
        return;
    }

    // extract color name from args
    char* color_name = args;
    char* end = args;
    while (*end && *end != ' ' && *end != '\t')
        end++;

    // null-terminate the color name
    if (*end)
        *end = '\0';

    // check colors
    if (strcmp(color_name, "black") == 0)            
        terminal_setbg_color(BLACK);
    else if (strcmp(color_name, "blue") == 0)        
        terminal_setbg_color(BLUE);
    else if (strcmp(color_name, "green") == 0)       
        terminal_setbg_color(GREEN);
    else if (strcmp(color_name, "cyan") == 0)        
        terminal_setbg_color(CYAN);
    else if (strcmp(color_name, "red") == 0)         
        terminal_setbg_color(RED);
    else if (strcmp(color_name, "magenta") == 0)     
        terminal_setbg_color(MAGENTA);
    else if (strcmp(color_name, "brown") == 0)       
        terminal_setbg_color(BROWN);
    else if (strcmp(color_name, "light_grey") == 0)  
        terminal_setbg_color(LIGHTGREY);
    else if (strcmp(color_name, "dark_grey") == 0)   
        terminal_setbg_color(DARKGREY);
    else if (strcmp(color_name, "light_blue") == 0)  
        terminal_setbg_color(LIGHTBLUE);
    else if (strcmp(color_name, "light_green") == 0) 
        terminal_setbg_color(LIGHTGREEN);
    else if (strcmp(color_name, "light_cyan") == 0)  
        terminal_setbg_color(LIGHTCYAN);
    else if (strcmp(color_name, "light_red") == 0)   
        terminal_setbg_color(LIGHTRED);
    else if (strcmp(color_name, "light_magenta") == 0) 
        terminal_setbg_color(LIGHTMAGENTA);
    else if (strcmp(color_name, "light_brown") == 0) 
        terminal_setbg_color(LIGHTBROWN);
    else if (strcmp(color_name, "white") == 0)       
        terminal_setbg_color(WHITE);
    
    else 
    {
        printf("Unknown color: %s\n", color_name);
        printf("Available: black blue green cyan red magenta brown light_grey dark_grey light_blue light_green light_cyan light_red light_magenta light_brown white\n");
        return;
    }

}

void repeat_text_cmd(char* args)
{
    // skip leading whitespace
    while (*args == ' ' || *args == '\t')
        args++;

    if (*args == '\0') 
    {
        printf("Usage: repeat [n] [text]\n");
        return;
    }

    // parse number
    int n = 0;
    char *p = args;
    while (*p >= '0' && *p <= '9') 
    {
        n = n * 10 + (*p - '0');
        p++;
    }

    // no digits 
    if (p == args) 
    {
        printf("Usage: repeat [n] [text]\n");
        return;
    }

    // skip whitespace before text
    while (*p == ' ' || *p == '\t')
        p++;

    // p now points to the text to repeat
    args = p;

    if (*args == '\0') 
    {
        // nothing to repeat
        return;
    }

    for (int i = 0; i < n; i++) 
    {
        printf("%s ", args);
    }
}

void exit_cmd(char* args)
{
    printf("Exiting shell...\n");

    shell_active = false;

    return;
}


#endif