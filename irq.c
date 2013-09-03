#include <irq.h>
#include <x86.h>
#include <idt.h>

/* Our external IRQ handlers */
extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

/* Currently a stubbed implementation since we don't really have to do IO waits */
PRIVATE /*STUB*/ void io_wait(void) {}

/* Remaps the PIC interrupt offsets. Making the master start at master_offset
 * and the slave start at slave_offset.
 * Typical values are master_offset = 32 and slave_offset = 40,
 * so that interrupts 0-47 are all used up (0-31 are CPU allocated)
 */
PRIVATE void PIC_remap(int master_offset, int slave_offset);

/* Sends the necessary End-of-interrupt by the given IRQ number.
 * NOTE: this number must be in the range 0-15 (IRQ 0, IRQ 1, etc..)
 * To convert from remapped interrupt numbers to this tange, use IRQ_NO
 */
PRIVATE void PIC_eoi(uint8_t irq_no);

PRIVATE void PIC_remap(int master_offset, int slave_offset) {
    unsigned char a1, a2;    
 
    /* Save the PIC masks so we can restore them later */
	a1 = inb(PIC1_DATA_PORT);                      
	a2 = inb(PIC2_DATA_PORT);
 
	outb(PIC1_COMMAND_PORT, ICW1_INIT+ICW1_ICW4);  /* starts the initialization sequence (in cascade mode)*/
	io_wait();
	outb(PIC2_COMMAND_PORT, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA_PORT, master_offset);           /* ICW2: Master PIC vector offset */
	io_wait();
	outb(PIC2_DATA_PORT, slave_offset);            /* ICW2: Slave PIC vector offset */
	io_wait();
	outb(PIC1_DATA_PORT, 4);                       /* ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100) */
	io_wait();
	outb(PIC2_DATA_PORT, 2);                       /* ICW3: tell Slave PIC its cascade identity (0000 0010) */
	io_wait();
 
	outb(PIC1_DATA_PORT, ICW4_8086);
	io_wait();
	outb(PIC2_DATA_PORT, ICW4_8086);
	io_wait();
 
    /* Restore the PIC masks */
	outb(PIC1_DATA_PORT, a1);
	outb(PIC2_DATA_PORT, a2);
}

PRIVATE void PIC_eoi(uint8_t irq_no)
{
    /* If it comes from the slave pic, we have to send it an EOI value too */
	if(irq_no >= 8)
		outb(PIC2_COMMAND_PORT,PIC_EOI);
 
	outb(PIC1_COMMAND_PORT,PIC_EOI);
}

void init_irq(void)
{
    /* Remap the PIC (so its interrupts/ISRs don't get confused with the CPU's */
	PIC_remap(32/*0x20*/,40/*0x28*/);
    
    /* Setup our irq handlers */
    idt_set_gate (IRQ_0/*32*/,  (uint32_t)irq0,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_1,        (uint32_t)irq1,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_2,        (uint32_t)irq2,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_3,        (uint32_t)irq3,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_4,        (uint32_t)irq4,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_5,        (uint32_t)irq5,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_6,        (uint32_t)irq6,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_7,        (uint32_t)irq7,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_8,        (uint32_t)irq8,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_9,        (uint32_t)irq9,  IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_10,       (uint32_t)irq10, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_11,       (uint32_t)irq11, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_12,       (uint32_t)irq12, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_13,       (uint32_t)irq13, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_14,       (uint32_t)irq14, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    idt_set_gate (IRQ_15/*47*/, (uint32_t)irq15, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
    
}

/* This gets called from our ASM interrupt handler stub. */
void irq_handler(registers_t* regs)
{
   idt_handler(regs);
   
   PIC_eoi(IRQ_NO(regs->int_no));
}
