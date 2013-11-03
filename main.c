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

/*
 * Kernel entry point
 */
int CDECL kernel_main ( multiboot_t* mboot_ptr )
{
    /*build_elf_symbols_from_multiboot(mboot_ptr); -- we'll get this working again later.. */
    uint32_t* ptr;
    UNUSED ( mboot_ptr );
    screen_clear();
    screen_puts ( "Hello World!\n" );

    set_fg_color ( VGA_YELLOW );
    screen_puts ( "Yellow!!\n" );
    set_bg_color ( VGA_BLUE );
    screen_puts ( "Yellow on blue!!\n" );
    set_fg_color ( VGA_WHITE );
    set_bg_color ( VGA_BLACK );

    init_gdt();
    screen_puts ( "GDT Started.\n" );

    init_idt();
    screen_puts ( "IDT Loaded.\n" );

    init_irq();

    screen_puts ( "IRQ Started!\n" );

    init_timer ( 1000 );

    screen_puts ( "Timer Started!\n" );

    init_keyboard();
   
    screen_put_hex ( ( uint32_t ) &kernel_main );
    screen_putc ( '\n' );
    ptr = ( uint32_t* ) 0xB0000000;
    screen_put_hex ( *ptr );
    screen_puts ( "\nShould now be 0xFFF: " );
    *ptr = 0xFFF;
    screen_put_hex ( *ptr );
    screen_puts ( "\n" );
    ptr += 1024 * 1024 - 1;
    screen_put_hex ( *ptr );

    init_pmm( 0xB0000000 );
    screen_puts ( "\nOkay, PMM enabled!\n" );
    init_vmm();
    screen_puts ( "\nOkay, VMM enabled!\n" );

    screen_put_hex ( ( uint32_t ) &kernel_main );
    screen_putc ( '\n' );
    ptr = ( uint32_t* ) 0xB0000000;
    screen_put_hex ( *ptr );
    screen_puts ( "\nShould now be 0xFFF: " );
    *ptr = 0xFFF;
    screen_put_hex ( *ptr );
    screen_puts ( "\n" );
    ptr += 1024 * 1024 - 1;
    screen_put_hex ( *ptr );
    screen_puts ( "\n" );
    
/*    pmm_initial_free_page_setup_HACK();*/
/*    screen_put_hex ( pmm_alloc_block() );*/
    screen_puts ( "\n" );
    __asm ( "sti" );
    for ( ;; ); /* NOTE: Never return from kernel, We'll segfault */
    return 0xDEADBABA; /* Should be in $eax right now */
}
