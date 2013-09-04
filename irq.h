#ifndef IRQ_H
#define IRQ_H
#include <stdinc.h>

/* All the PIC related ports */
#define PIC1_PORT		     0x20		/* IO base address for master PIC */
#define PIC2_PORT            0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND_PORT	 PIC1_PORT
#define PIC1_DATA_PORT	    (PIC1_PORT+1)
#define PIC2_COMMAND_PORT	 PIC2_PORT
#define PIC2_DATA_PORT	    (PIC2_PORT+1)

/* The value to send to a PIC (be it master or slave) to signal End-of-Interrupt
 * (whenever we're done processing)
 */
#define PIC_EOI		         0x20		/* End-of-interrupt command code */

/* Values to send to the PICs when initializing them... 
 * See http://wiki.osdev.org/PIC for more info */

#define ICW1_ICW4	         0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	         0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	     0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	         0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	         0x10		/* Initialization - required! */
 
#define ICW4_8086	         0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	         0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	     0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	     0x0C		/* Buffered mode/master */
#define ICW4_SFNM	         0x10		/* Special fully nested (not) */

/* The interrupt numbers for each of our IRQs */
#define IRQ_0  32
#define IRQ_1  33
#define IRQ_2  34
#define IRQ_3  35
#define IRQ_4  36
#define IRQ_5  37
#define IRQ_6  38
#define IRQ_7  39
#define IRQ_8  40
#define IRQ_9  41
#define IRQ_10 42
#define IRQ_11 43
#define IRQ_12 44
#define IRQ_13 45
#define IRQ_14 46
#define IRQ_15 47

/* Gets the IRQ number from an interrupt number. Example: for IRQ_0 returns 0,
 * and for IRQ_15 returns 15
 * 
 * FIXME: This currently assumes that the IRQs are remapped in the range 32-39
 * and 40-47, but if that changes, IRQ_NO changes as well
 */
#define IRQ_NO(x) ((x)-IRQ_0)

void init_irq(void);

#endif
