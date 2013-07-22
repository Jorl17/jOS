#include <idt.h>
#include <stdinc.h>
#include <mem.h>
#include <screen.h>

/* These extern directives let us access the addresses of our ASM ISR handlers. */
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr255();

extern void idt_set(uint32_t);

PRIVATE void idt_set_gate(uint8_t,uint32_t,uint16_t,uint8_t);
PRIVATE void div_by_zero(registers_t* regs);

#define NUM_IDTS 256


idt_entry_t idt_entries [NUM_IDTS];

idt_ptr_t idt_ptr;

interrupt_handler_t interrupt_handlers [NUM_IDTS];

/* Initialisation routine - zeroes all the interrupt service routines, and
   initialises the IDT. */
void init_idt ()
{
  /* Zero all interrupt handlers initially. */
  memset (&interrupt_handlers, 0, sizeof (interrupt_handler_t) * NUM_IDTS);

  /* Just like the GDT, the IDT has a "limit" field that is set to the last valid byte in the IDT,
    after adding in the start position (i.e. size-1). */
  idt_ptr.limit = sizeof (idt_entry_t) * NUM_IDTS - 1;
  idt_ptr.base  = (uint32_t) &idt_entries;

  /* Zero the IDT to start with. */
  memset (&idt_entries, 0, sizeof (idt_entry_t) * (NUM_IDTS-1));

  /* Set each gate in the IDT that we care about - that is:
     0-32:  Used by the CPU to report conditions, both normal and error.
   */
  idt_set_gate ( 0, (uint32_t)isr0 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate ( 1, (uint32_t)isr1 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate ( 2, (uint32_t)isr2 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate ( 3, (uint32_t)isr3 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate ( 4, (uint32_t)isr4 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate ( 5, (uint32_t)isr5 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate ( 6, (uint32_t)isr6 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate ( 7, (uint32_t)isr7 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate ( 8, (uint32_t)isr8 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate ( 9, (uint32_t)isr9 , IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (10, (uint32_t)isr10, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (11, (uint32_t)isr11, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (12, (uint32_t)isr12, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (13, (uint32_t)isr13, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (14, (uint32_t)isr14, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (15, (uint32_t)isr15, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (16, (uint32_t)isr16, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (17, (uint32_t)isr17, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (18, (uint32_t)isr18, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (19, (uint32_t)isr19, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (20, (uint32_t)isr20, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (21, (uint32_t)isr21, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (22, (uint32_t)isr22, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (23, (uint32_t)isr23, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (24, (uint32_t)isr24, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (25, (uint32_t)isr25, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (26, (uint32_t)isr26, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (27, (uint32_t)isr27, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (28, (uint32_t)isr28, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (29, (uint32_t)isr29, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (30, (uint32_t)isr30, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );
  idt_set_gate (31, (uint32_t)isr31, IDT_SELECTOR, IDT_32BIT_INTERRUPT_GATE );

  register_interrupt_handler(0,div_by_zero);
  /* Tell the CPU about our new IDT. */
  idt_set ((uint32_t)&idt_ptr);
}

PRIVATE void idt_set_gate (uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
  idt_entries[num].base_lo  = base & 0xFFFF;
  idt_entries[num].base_hi  = (base >> 16) & 0xFFFF;

  idt_entries[num].selector = sel;
  idt_entries[num].always0  = 0;
  
  /* We must uncomment the OR below when we get to using user-mode.
     It sets the interrupt gate's privilege level to 3. */
  idt_entries[num].flags    = flags /* | 0x60 */;
}

void idt_handler (registers_t* regs)
{
  if (interrupt_handlers [regs->int_no]) {
    interrupt_handlers [regs->int_no] (regs);
  } else {
    screen_puts ("Unhandled interrupt: ");
    screen_put_int(regs->int_no);
    screen_putc('\n');    
  }
}

void register_interrupt_handler (uint8_t n, interrupt_handler_t h)
{
  interrupt_handlers [n] = h;
}

PRIVATE void div_by_zero(registers_t* regs)
{
  UNUSED(regs);
  screen_puts ("Division by zero!\n");
  __asm("hlt");
}
