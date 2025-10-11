// defines the early memory configuration defines

#ifndef _MEM_H
#define _MEM_H

#define KERNEL_LOAD_PHYS   0x00100000 // 1MB
#define KERNEL_LOAD_VIRT   0xC0000000 // 3GB

/* low memoru below 640 KB for init kernel stack and heap */
#define KERNEL_STACK_EARLY 0x00090000 // 576KB
#define KERNEL_HEAP_EARLY  0x00000500 // grows up

#define KERNEL_HEAP_VIRT   0xC0100000 // 3GB + 1MB
#define KERNEL_HEAP_SIZE   0x00100000 // 1MB

/* configurable depending on some decision */
#define _IDENTITY_MAP_START 0x00000000 // 0
#define _IDENTITY_MAP_END   0x00400000 // 4MB

#endif