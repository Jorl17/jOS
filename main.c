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
#include <mem/pmm.h>
#include <mem/vmm.h>
#include "elf.h"
#include "multiboot.h"

/*
 * Kernel entry point
 */
int CDECL kernel_main ( multiboot_t* mboot_ptr )
{
    uint32_t mem;

    screen_clear();
    screen_puts ( "Hello World!\n" );

    set_fg_color ( VGA_YELLOW ); screen_puts ( "Yellow!!\n" );
    set_bg_color ( VGA_BLUE ); screen_puts ( "Yellow on blue!!\n" );
    set_fg_color ( VGA_WHITE ); set_bg_color ( VGA_BLACK );

    init_gdt(); screen_puts ( "GDT Started.\n" );
    init_idt(); screen_puts ( "IDT Loaded.\n" );
    init_irq(); screen_puts ( "IRQ Started!\n" );
    init_timer ( 1000 ); screen_puts ( "Timer Started!\n" );
    init_keyboard();

    screen_puts("kernel_main at "); screen_put_hex ( ( uint32_t ) &kernel_main ); screen_putc ( '\n' );

    init_pmm( 0xD0000000 ); screen_puts ( "PMM enabled!\n" );    
    init_vmm(mboot_ptr); screen_puts ( "VMM enabled!\n" );

    screen_puts ( "Adding physical frames to PMM ... " );
    mem = pmm_grab_physical_memory(mboot_ptr); screen_puts ( "added [" ); screen_put_uint(mem); screen_puts(" bytes / ~"); screen_put_uint(mem/(1024*1024)); screen_puts(" MB / ~");  screen_put_uint(mem/(1024*1024*1024)); screen_puts(" GB]\n");

    #if 0
    ptr = ( uint32_t* ) 0xD0000000;
    screen_put_hex ( *ptr );
    screen_puts ( "\nShould now be 0xFFF: " );
    *ptr = 0xFFF;
    screen_put_hex ( *ptr );
    screen_puts ( "\n" );
    ptr += 1024 * 1024 - 1;
    screen_put_hex ( *ptr );
    screen_puts ( "\n" );
    #endif

    screen_puts ( "Result of requesting physical PMM block: " ); screen_put_hex ( pmm_alloc_block(false) ); screen_puts ( "\n" );
    
    build_elf_symbols_from_multiboot(mboot_ptr); screen_puts("Kernel symbols loaded!\n");

    __asm ( "sti" );
    kpanic("Fuck yeah");
    for ( ;; ); /* NOTE: Never return from kernel, We'll segfault */
    return 0xDEADBABA; /* Should be in $eax right now */
}
