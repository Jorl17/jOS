#ifndef INTERNAL_TIMER_H
#define INTERNAL_TIMER_H
#include <stdinc.h>
/*
 * To have timer support, we're going to be using the PIT.
 * 
 * The PIT is a neat chip which has 5 operating modes, and three channels.
 * These channels are independent. Channel 0 is connected to IRQ0,
 * channel 1 is no longer used (not even implemented on modern hardware)
 * and channel 2 is connected to the PC's internal speaker.
 * 
 * Thus, the PIT operates with frequencies -- it is an osscilator and a prescaler.
 * In the case of Channel 0, it might be useful to periodically send interrupts
 * (it's what we'll do), and in the case of channel 2, to generate frequencies
 * to pulse the computer speaker at, thus making sound.
 * 
 * The PIT also has an internal counter, which can be accessed,
 * but we won't be focusing on that right now. For more info, check
 * http://wiki.osdev.org/PIT
 * 
 * For timer support, we'll be focusing on the first channel.
 * 
 * Each channel has 5 different operating modes, which can be checked at
 * http://wiki.osdev.org/PIT#Operating_Modes .
 * 
 * The basic idea, though, is that:
 *  -> Mode 0, only available to channel 0, waits for software (the OS)
 *     to set the counter limit. Then, as the PIC ticks, it is decremented
 *     each time. When it gets to 0, the line out is put at 1 until the PIT
 *     is reset (example: a new value is written)
 *  -> Mode 1 can't be used with channel 0 so we'll skip the explanation
 *  -> Mode 2 operates as a "Rate Generator". We set an interval value
 *     and the PIC generates us pulses with that interval
 *  -> Mode 3 is similar to Mode 2, but, instead of sending pulses,
 *     operates with a square wave, thus meaning that the output stays
 *     in the high/1 state for longer than in Mode 2. It is usually used
 *     for timers, and it is what we will use. Mode 2 can also be used,
 *     since it it allows for higher frequencies (as 50% of the wave isn't
 *     being used for the square wave)
 *  -> Mode 4 and Mode 5 are used for strobes and you can check them out at
 *     the aforementioned page.
 * 
 * 
 * 
 * To get timer support, we will use the PIT mode 3 on channel 0, generating square
 * waves periodically.
 * 
 * To do this, we will first send to "Channel 0" the PIT_MASK_TIMER byte
 * which has all the right flags. Then we'll have to send the high and low bytes
 * to the PIT_COMMAND_DATA_PORT. This means that the value we send is 2-bytes,
 * and it is, contrary to what could be expected, the frequency itself. We send
 * the divisor of 1193180 by the given frequency.
 */

/* The default frequency, which we have to divide by the target frequency
 * to get the value to send to the PIT command data port
 */
#define PIT_DEFAULT_FREQ 1193180
 
/* The I/O ports for the PIC */
#define PIT_CHANNEL0_DATA_PORT    0x40
#define PIT_CHANNEL1_DATA_PORT    0x41
#define PIT_CHANNEL2_DATA_PORT    0x42
#define PIT_COMMAND_DATA_PORT     0x43

/* To set the operation mode of a channel, we have to send a command
 * byte. This byte can be built by logically or-ing the desired
 * PIT_MASK_* options in here listed.
 */
 
/* Channel selection -> We'll use 0 (connected to IRQ0)*/
#define PIT_MASK_CHANNEL0         0x00
#define PIT_MASK_CHANNEL1         0x40
#define PIT_MASK_CHANNEL2         0x80

/* Access type -> We'll use LOHIBYTES (we send a full 16-bit divisor) */
#define PIT_MASK_ACCESS_LATCH     0x00
#define PIT_MASK_ACCESS_LOBYTE    0x10
#define PIT_MASK_ACCESS_HIBYTE    0x20
#define PIT_MASK_ACCESS_LOHIBYTES 0x30

/* Operation mode -> We'll use 3, though 2 would probably be okay too */
#define PIT_MASK_MODE_0           0x00
#define PIT_MASK_MODE_1           0x02
#define PIT_MASK_MODE_2           0x04
#define PIT_MASK_MODE_3           0x06
#define PIT_MASK_MODE_4           0x08
#define PIT_MASK_MODE_5           0x0A

/* We go with binary and enough talking about it */
#define PIT_MASK_BCD              0x01
#define PIT_MASK_BINARY           0x00

/* The flags that our timer will use to setup the PIT correctly for us */
#define PIT_MASK_TIMER (PIT_MASK_CHANNEL0 | PIT_MASK_ACCESS_LOHIBYTES | PIT_MASK_MODE_3 | PIT_MASK_BINARY)



void init_timer(uint32_t frequency_hz);
double get_time_elapsed_since_boot(void);

#endif
