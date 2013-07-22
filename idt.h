#ifndef IDT_H
#define IDT_H

#include <stdinc.h>

void init_idt ();

/*
 * The IDT holds entries stating what to do when an interrupt happens.
 * Each of these entries must hold a BASE address, a SELECTOR and some specific
 * flags. The SELECTOR+BASE system works like segmentation, finding the selector
 * and then adding the base address to the address it first covers.
 * FIXME: Check if the previous sentence aplies to IDT Gates other than
 *        Interrupt Gates (I'm _sure_ it's the same for Trap gates) .
 * In a FLAT memory model, any selector will do, as long as it has executable
 * rights. In the case of our kernel, we use a FLAT memory model and the code
 * selector is used to set CS.
 */

/* This structure describes one interrupt gate. */
typedef struct
{
  uint16_t base_lo;        /* The lower 16 bits of the address to jump to. */
  uint16_t selector;       /* Kernel segment selector. See below. */
  uint8_t  always0;        /* This must always be zero. */
  uint8_t  flags;          /* More flags, also called TYPE attribute. See below. */
  uint16_t base_hi;        /* The upper 16 bits of the address to jump to. */
} __attribute__((packed)) idt_entry_t;

/* The selector we want is the code selector, which is at offset 0x08 (we set it up like that in the GDT) */
/* FIXME: Make sure we really get this....We're missing on something! */
#define IDT_SELECTOR 0x08

/*
 * Format of flags/type attribute field:
   ---------------------------------------------------------------------------------
   | Present bit (can be 0 for paging) | Ring level (0-3) |  Storage  |    Type    |
   |               1 bit               |      2 bits      |   1 bit   |   4 bits   |
   ---------------------------------------------------------------------------------

  The ring level is used for protection mechanisms. it states what is the 
  minimum ring level for the code to generate this interrupt.
  
  The type distinguishes between Task gates, Interrupt Gates and Trap gates,
  in 16-bit and 32-bit variations (there are 5 possible values, see Intel docs.)
  It is usually set to an INTERRUPT GATE (type = 1110). This kind gate should
  be used for ISRs and, like the TRAP GATE, pushes EFLAGS, CS and EIP on the
  stack (something we exploit in idt_s.s to build the regs_t structure). The
  interrupt gate also disables interrupts when entering the routine, and then
  reenables them.
  
  
  The storage bit should be set to 0 for interrupt gates. FIXME: And other
  scenarios?
   
 */
 
/*                  P    R S Type          */
/* 32bit Interrupt: 1  000 0 1110 --> 0x8E */
/* 32bit Trap:      1  000 0 1111 --> 0x8F */
#define IDT_32BIT_INTERRUPT_GATE 0x8E
#define IDT_32BIT_TRAP_GATE      0x8F 

/* A pointer structure used for informing the CPU about our IDT. */
typedef struct
{
  uint16_t limit;
  uint32_t base;           /* The address of the first element in our idt_entry_t array. */
} __attribute__((packed)) idt_ptr_t;

/* Structure containing register values when the CPU was interrupted. */
typedef struct
{
  uint32_t ds;             /* Data segment selector. Stored in lower 16 bits (to use later)*/
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha. */
  uint32_t int_no, err_code; /* Interrupt number and error code (if applicable). */
  uint32_t eip, cs, eflags, useresp, ss; /* Pushed by the processor automatically (this is because we use Interrupt Gates, for which the processor does that. */
} registers_t;

/* An interrupt handler. It is a pointer to a function which takes a pointer 
   to a structure containing register values. */
typedef void (*interrupt_handler_t)(registers_t *);

/* Allows us to register an interrupt handler. */
void register_interrupt_handler (uint8_t n, interrupt_handler_t h);
#endif
