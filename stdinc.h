#ifndef STDINC_H
#define STDINC_H
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

#define PRIVATE static

#endif
