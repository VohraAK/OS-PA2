#ifndef _SYSCALL_C
#define _SYSCALL_C

#include <init/syscall.h>

// define a dispatch table
void* syscall_dispatch_table[NUM_SYSCALLS];

// user space
void read(char* buffer, uint32_t size)
{
    // register syscall number 0 into eax
    // first arg in ebx
    // second in ecx
    // interrupt kernel at vector 0x80

    // inline code written with help from LLMs
    asm volatile
    (
        "movl $0, %%eax;" // syscall number 0
        "movl %0, %%ebx;" // buffer
        "movl %1, %%ecx;" // size
        "int $0x80;" // interrupt
        :
        : "r"(buffer), "r"(size)
        : "eax", "ebx", "ecx"
    );

}

void write(const char* buffer, uint32_t size)
{
    asm volatile
    (
        "movl $1, %%eax;" // syscall number 1
        "movl %0, %%ebx;" // buffer
        "movl %1, %%ecx;" // size
        "int $0x80;" // interrupt
        :
        : "r"(buffer), "r"(size)
        : "eax", "ebx", "ecx"
    );
}

void clear(void)
{
    // syscall to clear screen
    asm volatile
    (
        "movl $2, %%eax;" // syscall 2
        "int $0x80;"
        :
        :
        : "eax"
    );
}


// -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

// kernel space

void syscall_handler(interrupt_context_t *context)
{
    // welcome to the kernel side
    
    // extract the contents of registers
    uint32_t syscall_number = context->eax;
    
    // check range of eax
    if (syscall_number >= NUM_SYSCALLS)
        return;
    
    // get function address
    void* func = syscall_dispatch_table[syscall_number];

    if (!func)
        return;
    
    uint32_t size = context->ecx;
    
    // cast it into appropriate func ptr type
    switch (syscall_number)
    {
        case SYSCALL_READ:
            read_func_t read_func = (read_func_t)func;
        
            // extract args from context
            char* buffer = (char* )context->ebx;
        
            // call da func ;)
            read_func(buffer, size);
        
            break;
        
        case SYSCALL_WRITE:
            write_func_t write_func = (write_func_t)func;
            
            // extract args from context
            const char* data = (const char*) context->ebx;
            
            // call da func!! ;) ;)
            write_func(data, size);
        
            break;
        
        case SYSCALL_CLEAR:
            generic_void_func_t clear_func = (generic_void_func_t)func;

            clear_func();

            break;

    }
    
}

void syscall_init(void)
{
    // register syscall handler at 0x80
    register_interrupt_handler(128, (interrupt_service_t)syscall_handler);

    // set up dispatch table
    syscall_dispatch_table[SYSCALL_READ] = (void *)&terminal_read;
    syscall_dispatch_table[SYSCALL_WRITE] = (void *)&terminal_write;
    syscall_dispatch_table[SYSCALL_CLEAR] = (void *)&terminal_clear_scr;

}

#endif