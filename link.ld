/* Link.ld -- Linker script for the kernel - ensure everything goes in the */
/*            Correct place.  */
/*            Original file taken from Bran's Kernel Development */
/*            tutorials: http://www.osdever.net/bkerndev/index.php. */

/*
  All sections are page-aligned (4kb-aligned)
 */

/*
  The addresses of code, bss and end are used in start.s to tell GRUB where each
  section is.
*/
ENTRY(start)
SECTIONS
{

    /* Start at 1MB. 0x0-0x0FFFFF is reserved and GRUB can't even load us there.
       As per http://www.brokenthorn.com/Resources/OSDev7.html
    */
    .text 0x100000 :
    {
        code = .; _code = .; __code = .;
        *(.text)
        . = ALIGN(4096);
    }

    .data :
    {
        data = .; _data = .; __data = .;
        *(.data)
        *(.rodata)
        . = ALIGN(4096);
    }

    .bss :
    {
        bss = .; _bss = .; __bss = .;
        *(.bss)
        . = ALIGN(4096);
    }

    end = .; _end = .; __end = .;
}
