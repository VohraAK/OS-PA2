#ifndef _PIC_H
#define _PIC_H

#include <stdint.h>

//----------------------------
// constants for the PIC ports
//----------------------------

#define MASTER_PIC_COMMAND_PORT  0x20  // Command port for the master PIC
#define MASTER_PIC_DATA_PORT     0x21  // Data port for the master PIC
#define SLAVE_PIC_COMMAND_PORT   0xA0  // Command port for the slave PIC
#define SLAVE_PIC_DATA_PORT      0xA1  // Data port for the

// -------------------------------
// defines for programming the PIC
// -------------------------------

//! Initialization Command Word 1 (ICW1) bits
#define PIC_ICW1_EXPECT_ICW4        0x01  // Expect ICW4
#define PIC_ICW1_SINGLE             0x02  // Single PIC mode
#define PIC_ICW1_CALL_ADD_INTERVAL  0x04  // Call address interval bit, ignored on x86
#define PIC_ICW1_LEVEL_TRIGGERED    0x08  // Level triggered mode (if not set, edge triggered mode)
#define PIC_ICW1_START_INIT         0x10  // Start initialization

/* for the sake of completeness, the other ICW1 bits are included here, 
	although they are not used on the x86 (must be defaulted to 0) */
#define _PIC_ICW1_MCS8085_IVT0     	0x20  // MCS-8085 IVT0
#define _PIC_ICW1_MCS8085_IVT1     	0x40  // MCS-8085 IVT1
#define _PIC_ICW1_MCS8085_IVT2     	0x60  // MCS-8085 IVT2

//! Initialization Command Word 2 (ICW2) bits
// ICW2 must be sent 
#define PIC_ICW2_IVT_BASE(base) ((base) & 0xFF)  // IVT base address

//! Initialization Command Word 3 (ICW3) bits
// ICW3 must be sent after sending an ICW1 with cascading enabled (ICW1_SINGLE not set)

// for the master PIC, this is the pin number of the slave PIC
#define _ICW3_PRIM_MASK 			0xFF  // Mask for the master PIC pins (bits 0-2)
#define PIC_ICW3_PRIM_SLAVE_PIN(pin) ((1 << (pin)) & _ICW3_PRIM_MASK)  // Primary PIC slave pin

// for the slave PIC, this is the pin number of the master PIC
#define _ICW3_SEC_MASK 				0x07  // Mask for the slave PIC pins (bits 0-2)
#define PIC_ICW3_SEC_MASTER_PIN(pin) ((pin) & _ICW3_SEC_MASK)  // Secondary PIC master pin

// Initialization Command Word 4 (ICW4) bits
#define PIC_ICW4_8086_MODE          0x01  // 8086/88 mode (if not set, 8080/85 mode)
#define PIC_ICW4_AUTO_EOI           0x02  // Auto End of Interrupt (EOI) mode (controller automatically sends EOI after handling an interrupt)
#define PIC_ICW4_MASTER_BUF         0x04  // only for master PIC, and if BUF is set, this bit is set to indicate that the master PIC is buffered
#define PIC_ICW4_BUFFERED_MODE      0x08  // Buffered mode (if not set, non-buffered mode)
#define PIC_ICW4_SFNM               0x10  // Special Fully Nested Mode (if not set, normal mode)

// Operation Command Word (OCW2) defines
#define PIC_OCW2_ROT        0x80
#define PIC_OCW2_SEL        0x40
#define PIC_OCW2_EOI        0x20
#define PIC_OCW2_LEVEL(x)   ((x) & 0x07)

/**
 * @brief Initializes the 8259 Programmable Interrupt Controller (PIC) for x86 architecture.
 * 
 * @param master_base The base address for the master PIC (usually 0x20).
 * @param slave_base The base address for the slave PIC (usually 0x28).
 */
void pic_init (uint8_t master_base, uint8_t slave_base);

/**
 * @brief Sends an End of Interrupt (EOI) signal to the PIC for the specified IRQ.
 * 
 * @param irq The IRQ number for which to send the EOI signal.
 */
void pic_send_eoi (uint8_t irq);

#endif // !_PIC_H