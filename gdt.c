#include <gdt.h>

#define NUM_ENTRIES 3

extern void gdt_set (uint32_t);
PRIVATE void gdt_set_gate (int32_t,uint32_t,uint32_t,uint8_t,uint8_t);

/* GDT Entries */
gdt_entry_t gdt_entries [NUM_ENTRIES];

/* Our GDT Pointer (remember the x86 needs a special structure to store the
   location of the GDT entries */
gdt_ptr_t gdt_ptr;

void init_gdt ()
{
  gdt_ptr.limit = sizeof (gdt_entry_t) * NUM_ENTRIES - 1;

  gdt_ptr.base = (uint32_t) &gdt_entries;       

  gdt_set_gate (0, 0, 0, 0, 0);                                                     /* Null segment. */
  gdt_set_gate (1, FLATMODEL_BASE, FLATMODEL_LIMIT, CODE_SELECTOR, FLATMODEL_GRAN); /* Code segment. */
  gdt_set_gate (2, FLATMODEL_BASE, FLATMODEL_LIMIT, DATA_SELECTOR, FLATMODEL_GRAN); /* Data segment. */

  gdt_set ((uint32_t) &gdt_ptr);
}

PRIVATE void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    /* Make sure that granularity has only the right bits set */
    gran &= 0xF0;

    /* Split the base address in the required fields */
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    /* Split the limit address in the required fields.
       Note that limit_low is in the granularity field. */
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    
    /* Set granularity options */
    gdt_entries[num].granularity |= gran;
    
    /* Set access flags */
    gdt_entries[num].access      = access;
}
