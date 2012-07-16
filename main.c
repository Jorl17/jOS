
/**
 * The CDECL macro is used to force the compiler to choose the cdecl calling
 * convention. http://en.wikipedia.org/wiki/X86_calling_conventions#cdecl 
 */
#if defined(__GNUC__) || defined(__clang__)
/* GCC & CLANG */
#define CDECL __attribute__((cdecl))
#elif defined(_MSC_VER)
/* MSVC */
#define CDECL __cdecl
#endif

/**
 * Go around some ansi/pedantic warnings with unused parameters
 * __JOS_PEDANTIC is meant for us to define
 */
#ifdef __JOS_PEDANTIC
#define UNUSED(x) ((x)=(x))
#else
#define UNUSED(x) do {} while(0)
#endif

/*
 * Kernel entry point
 */
int CDECL kernel_main(void* mboot_ptr)
{
  UNUSED(mboot_ptr);
  return 0xDEADBABA; /* Should be in $eax right now */
}
