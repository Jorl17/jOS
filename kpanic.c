#include <stdinc.h>
#include <kpanic.h>
#include <screen.h>
#include <elf.h>

PRIVATE void print_stack_trace ();

void kpanic (const char* msg)
{
  __asm("cli");
  screen_puts ("!!!-> System panic: ");
  screen_puts ( msg );
  screen_putc('\n');
  print_stack_trace ();
  screen_puts ("!!!->\n");
  for (;;) ;
}

void print_stack_trace ()
{
  uint32_t* ebp,
          * eip;
  __asm volatile ("mov %%ebp, %0" : "=r" (ebp));
  for (; ebp; ebp = (uint32_t*) *ebp) {
    eip = ebp+1;
    screen_puts("    :: ");
    screen_put_hex(*eip);
    screen_putc(' ');
    screen_puts( kernel_elf_lookup_symbol_function (*eip) );
    screen_putc('\n');    
  }
}
