
/**
 * The CDECL macro is used to force the compiler to choose the cdecl calling
   convention. http://en.wikipedia.org/wiki/X86_calling_conventions#cdecl 
 */
#ifdef __GNUC__
/* GCC */
#define CDECL __attribute__((cdecl))
#elif defined(_MSC_VER)
/* MSVC */
#define CDECL __cdecl
#endif

int CDECL main(void* mboot_ptr)
{
  /* Kernel entry point */
  return 0xDEADBABA; /* cdecl should be in $eax right now */
}
