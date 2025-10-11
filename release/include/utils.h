#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief an inline function to write a byte to the specified port.
 * 
 * @param value 8-bit value to write to the port.
 * @param port Max 16-bit port number.
 */
static inline void outb (uint8_t value, uint16_t port) {
    asm volatile (
        "outb %0, %1"            /* specifies the instruction */
        :                        /* no output */
        : "a"(value), "dN"(port) /* input operands: value in register a (al),
                                    port in register d (dx). */
                                 /* encodings possible:
                                    outb %al, $imm8 or outb %al, %dx */
    );   
}

/**
 * @brief an inline function to read a byte from the specified port.
 * 
 * @param port Max 16-bit port number.
 * @return 8-bit value read from the port.
 */
static inline uint8_t inb (uint16_t port) {
    uint8_t ret;
    asm volatile (
        "inb %1, %0"        /* specifies the instruction */
        : "=a"(ret)         /* output operand: value in register a (al) */
        : "dN"(port)        /* input operand: port in register d (dx). */
                            /* encodings possible: inb %dx, %al or inb $imm8, %al */
    );
    return ret;
}

/**
 * @brief an inline function to write a word (16 bits) to the specified port.
 * 
 * @param value 16-bit value to write to the port.
 * @param port Max 16-bit port number.
 */
static inline void outw (uint16_t value, uint16_t port) {
    asm volatile (
        "outw %0, %1"            /* specifies the instruction */
        :                        /* no output */
        : "a"(value), "dN"(port) /* input operands: value in register a (ax),
                                    port in register d (dx). */
    );
}

/**
 * @brief an inline function to read a word (16 bits) from the specified port.
 * 
 * @param port Max 16-bit port number.
 * @return 16-bit value read from the port.
 */
static inline uint16_t inw (uint16_t port) {
    uint16_t ret;
    asm volatile (
        "inw %1, %0"        /* specifies the instruction */
        : "=a"(ret)         /* output operand: value in register a (ax) */
        : "dN"(port)        /* input operand: port in register d (dx). */
    );
    return ret;
}

/**
 * @brief an inline function to write a block of data to the specified port.
 * 
 * @param port Max 16-bit port number.
 * @param addr Pointer to the data to write.
 * @param count Number of bytes to write.
 */
static inline void outsb (uint16_t port, const void* addr, size_t count) {
    asm volatile (
        "rep outsb"
        : "+S"(addr), "+c"(count)   /* buffer start in ESI, ECX = count */
        : "d" (port)                /* destination port in DX */
    );
}

/**
 * @brief an inline function to read a block of data from the specified port.
 * 
 * @param port Max 16-bit port number.
 * @param addr Pointer to the buffer to read data into.
 * @param count Number of bytes to read.
 */
static inline void insb (uint16_t port, void* addr, size_t count) {
    asm volatile (
        "rep insb"
        : "+D"(addr), "+c"(count)   /* buffer start in EDI, ECX = count */
        : "d" (port)                /* source port in DX */
        : "memory"  /* clobbers memory, as it may be modified */
    );
}

/**
 * @brief an inline function to write a block of words (16 bits) to the
 * specified port.
 * 
 * @param port Max 16-bit port number.
 * @param addr Pointer to the data to write.
 * @param count Number of words to write.
 */
static inline void outsw (uint16_t port, const void* addr, size_t count) {
    asm volatile (
        "rep outsw"
        : "+S"(addr), "+c"(count)   /* buffer start in ESI, ECX = count */
        : "d" (port)                /* destination port in DX */
    );
}

/**
 * @brief an inline function to read a block of words (16 bits) from the
 * specified port.
 * 
 * @param port Max 16-bit port number.
 * @param addr Pointer to the buffer to read data into.
 * @param count Number of words to read.
 */
static inline void insw (uint16_t port, void* addr, size_t count) {
    asm volatile (
        "rep insw"
        : "+D"(addr), "+c"(count)   /* buffer start in EDI, ECX = count */
        : "d" (port)                /* source port in DX */
        : "memory"  /* clobbers memory, as it may be modified */
    );
}

// aligns the given address addr to the provided alignment
// useful for generating e.g. page aligned addresses
#define ALIGN(addr, alignment) \
    ((void*)(((uintptr_t)(addr) + (alignment) - 1) & ~((alignment) - 1)))

// aligns a size to the provided alignment
#define ALIGN_SIZE(size, alignment) \
    (((size) + (alignment) - 1) & ~((alignment) - 1))

// simply checks whether the given address follows a particular alignment or not
#define IS_ALIGNED(addr, alignment) \
    (((uintptr_t)(addr) & ((alignment) - 1)) == 0)


//! disables interrupts globally
static inline void cli(void) {
    asm volatile ("cli" :::);
}

//! enables interrupts globally
static inline void sti(void) {
    asm volatile ("sti" :::);
}


//! macro to get esp value into specified var
#define GET_ESP(var) \
    asm volatile ("mov %%esp, %0" : "=r"(var))

//! macro to get ebp value into specified var
#define GET_EBP(var) \
    asm volatile ("mov %%ebp, %0" : "=r"(var))


//! macros to get low and high bytes of a 16 bit value
#define LOW_BYTE(x)  ((uint8_t)((x) & 0xFF))
#define HIGH_BYTE(x) ((uint8_t)(((x) >> 8) & 0xFF))

#endif // _UTILS_H