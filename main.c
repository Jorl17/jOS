#include <stdinc.h>
#include <screen.h>
#include <gdt.h>
#include <idt.h>
#include <irq.h>
#include <internal_timer.h>
#include <multiboot.h>
#include <kpanic.h>
#include <elf.h>
#include <keyboard.h>

/*
 * Kernel entry point
 */
int CDECL kernel_main(multiboot_t* mboot_ptr)
{
  /*build_elf_symbols_from_multiboot(mboot_ptr); -- we'll get this working again later.. */
  UNUSED(mboot_ptr);
  screen_clear();
  screen_puts("Hello World!\n");
  
  set_fg_color(VGA_YELLOW);
  screen_puts("Yellow!!\n");
  set_bg_color(VGA_BLUE);
  screen_puts("Yellow on blue!!\n");
  set_fg_color(VGA_WHITE);  set_bg_color(VGA_BLACK);

  init_gdt();
  screen_puts("GDT Started.\n");  
  
  init_idt();
  screen_puts("IDT Loaded.\n");
  
  init_irq();
  
  screen_puts("IRQ Started!\n");
  
  init_timer(1000);
  
  screen_puts("Timer Started!\n");
  
  init_keyboard();
  
  screen_put_hex((uint32_t)&kernel_main);
  screen_putc('\n');
  
  __asm("sti");
  for(;;); /* NOTE: Never return from kernel, We'll segfault */
  return 0xDEADBABA; /* Should be in $eax right now */
}
