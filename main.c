#include <stdinc.h>
#include <screen.h>
/*
 * Kernel entry point
 */
int CDECL kernel_main(void* mboot_ptr)
{
  UNUSED(mboot_ptr);
  screen_putc('h');
  return 0xDEADBABA; /* Should be in $eax right now */
}
