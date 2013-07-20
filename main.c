#include <stdinc.h>
#include <screen.h>
/*
 * Kernel entry point
 */
int CDECL kernel_main(void* mboot_ptr)
{
  UNUSED(mboot_ptr);
  screen_clear();
  screen_puts("Hello World!\n");
  set_fg_color(VGA_YELLOW);
  screen_puts("Yellow!!\n");
  set_bg_color(VGA_BLUE);
  screen_puts("Yellow on blue!!\n");
  return 0xDEADBABA; /* Should be in $eax right now */
}
