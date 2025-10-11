#ifndef _IDT_C
#define _IDT_C

#include "init/idt.h"

idt_entry_t idt_entries[IDT_TABLE_SIZE];
idt_ptr_t idt_ptr;

void create_idt_entry(idt_entry_t *entry, uint32_t offset, uint16_t selector, uint8_t gate_type_attr)
{
    // split offset into high and low 16-bits
    uint16_t offset_low_16 = (offset) & 0xFFFF;
    uint16_t offset_high_16 = (offset >> 16) & (0xFFFF);

    // it's setting time :)
    entry->offset_high_16 = offset_high_16;
    entry->offset_low_16 = offset_low_16;
    entry->selector = selector;

    entry->reserved = (uint8_t) 0;
    entry->gate_type_attr = gate_type_attr;

}

void delete_idt_entry(idt_entry_t *entry)
{
    entry->offset_low_16 = 0;
    entry->offset_high_16 = 0;
    entry->selector = 0;
    entry->reserved = 0;
    entry->gate_type_attr = 0;
}


void idt_init()
{
    // initialise an IDT table of size (256*8 = 2048 bytes)
    idt_ptr.limit = (int) (IDT_TABLE_SIZE*IDT_ENTRY_SIZE) - 1;
    idt_ptr.base = (uint32_t) &idt_entries;

    // create IDT entries for CPU exceptions (0-37) with exception gate
    for (uint8_t i = 0; i < 48; i++)
        create_idt_entry(&idt_entries[i], (uint32_t)isr_stubs[i], SEGMENT_SELECTOR, IDT_32BIT_EXCEPTION_GATE);
    
    // create IDT entry for software-defined interrupts
    create_idt_entry(&idt_entries[128], (uint32_t)isr128, SEGMENT_SELECTOR, IDT_SYSCALL_GATE);

    // load IDT into CPU
    load_idt((uint32_t) &idt_ptr);

}

#endif