#ifndef _SYS_INTERRUPTS_C
#define _SYS_INTERRUPTS_C

#include "interrupts.h"

interrupt_service_t handler_addresses[IDT_TABLE_SIZE];

void interrupt_dispatch (interrupt_context_t* context)
{
    // the context has been already built in isr_common_handler
    // read the interrupt number in the context
    uint32_t n = context->interrupt_number;

    // check if it is a hardware interrupt (32-48)
    if (n >= 32 && n <= 47)
    {
        // send EOI to PIC
        pic_send_eoi(n);
    }

    interrupt_service_t handler = get_interrupt_handler(n);

    if (handler == (interrupt_service_t)0)
        return;
    
    // handler is present, call da function
    handler(context);
}

void register_interrupt_handler(uint8_t interrupt_number, interrupt_service_t handler)
{
    // check if the corresponding entry exists in the IDT entry table
    // check both the handler/offset and the present bit
    uint8_t status = check_idt_entry(interrupt_number);

    if (!status)
        // IDT entry does not exist!
        return;
    
    handler_addresses[interrupt_number] = handler;
    
}

void unregister_interrupt_handler (uint8_t int_no)
{
    // check if the corresponding entry exists in the IDT entry table
    // check both the handler/offset and the present bit
    uint8_t status = check_idt_entry(int_no);

    if (!status)
        return;
    
    // erase the entry
    handler_addresses[int_no] = 0;

}

// helper function
uint8_t check_idt_entry(uint8_t int_no)
{
    uint32_t handler_value = (idt_entries[int_no].offset_high_16 << 16) | (idt_entries[int_no].offset_low_16);
    uint32_t present = (idt_entries[int_no].gate_type_attr & 0x80);

    if (handler_value == 0 || !present)
        // interrept handler not present/registered
        return (uint8_t)0;
    
    return (uint8_t)1;
}


interrupt_service_t get_interrupt_handler (uint8_t int_no)
{
    // check if the corresponding entry exists in the IDT entry table
    // check both the handler/offset and the present bit
    uint8_t status = check_idt_entry(int_no);

    if (!status)
        return 0;
    
    interrupt_service_t handler = handler_addresses[int_no];

    return handler;
}

void setup_x86_interrupts()
{
    // init PIC
    pic_init(PIC_MASTER_BASE, PIC_SLAVE_BASE);
    
    // init IDT
    idt_init();

    // set Interrupt Flag in EFLAGS
    sti();
}

#endif
