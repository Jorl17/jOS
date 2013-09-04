#include <internal_timer.h>
#include <idt.h>
#include <irq.h>
#include <screen.h>

/* FIXME: Maybe 64? */
uint32_t num_ticks = 0;
uint32_t sysfrequency_hz = PIT_DEFAULT_FREQ;

PRIVATE void timer_callback ( registers_t* regs );

PRIVATE void timer_callback ( registers_t* regs )
{
    UNUSED ( regs );
    num_ticks++;
    if ( num_ticks % 1000 == 0 ) {
        screen_puts ( "One second has passed!\n" );
    }

    if ( 0 ) {
        screen_puts ( "Tick: " );
        screen_put_int ( num_ticks );
        screen_putc ( '\n' );
    }

}

void init_timer ( uint32_t frequency_hz );

double get_time_elapsed_since_boot ( void )
{
    return ( ( double ) num_ticks ) / sysfrequency_hz;
}

void init_timer ( uint32_t frequency )
{
    uint32_t divisor;
    uint8_t l, h;
    /* Start by registering the new interrupt handler */
    register_interrupt_handler ( IRQ_0, &timer_callback );

    /* The value to send to the PIT is the one to divide 1193180
     * It must also fit in 16 bits, otherwise it goes boo-boo...
     */
    divisor = PIT_DEFAULT_FREQ / frequency;

    /* Send the command byte. */
    outb ( PIT_COMMAND_DATA_PORT, PIT_MASK_TIMER );

    /* Split the divisor in lower and higher bytes, and send them */
    l = ( uint8_t ) ( divisor & 0xFF );
    h = ( uint8_t ) ( ( divisor >> 8 ) & 0xFF );

    outb ( PIT_CHANNEL0_DATA_PORT, l );
    outb ( PIT_CHANNEL0_DATA_PORT, h );
}
