#include <keyboard.h>
#include <idt.h>
#include <irq.h>
#include <x86/x86.h>
#include <screen.h>

PRIVATE keyboard_map_t* system_map;

/* Given a VK, we access the respective index (for VK_1 we access index 3)
 * to get that key's state (or set it) */
PRIVATE uint8_t  key_states[128] = {0};

PRIVATE byte     vk_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
    '9', '0', '-', '=', '\b',	/* Backspace */
    '\t',			/* Tab */
    'q', 'w', 'e', 'r',	/* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
    '\'', '`',   0,		/* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
    'm', ',', '.', '/',   0,				/* Right shift */
    '*',
    0,	/* Alt */
    ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
    '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
    '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};


#define ALT_PRESSED()    IS_KEY_DOWN(key_states[VK_ALT])
#define CTRL_PRESSED()   IS_KEY_DOWN(key_states[VK_CTRL])
#define LSHIFT_PRESSED() IS_KEY_DOWN(key_states[VK_LSHIFT])
#define CAPS_ON()        IS_KEY_DOWN(key_states[VK_CAPS])

#define KEY_RELEASED(x) ( ((x) & KEY_RELEASED_MASK) == KEY_RELEASED_MASK )

#define SET_KEY_DOWN(x) ( (x) |= KEY_DOWN_STATE )
#define SET_KEY_UP(x)   ( (x) &= ~KEY_DOWN_STATE )

/* Handles the keyboard interrupt */
void keyboard_handler ( registers_t* r )
{
    byte scancode;
    uint16_t vk_code;

    UNUSED(r);
    /* Read from the keyboard's data buffer */
    scancode = inb ( KEYB_DATA_PORT );

    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if ( KEY_RELEASED ( scancode ) ) {
        scancode &= ~KEY_RELEASED_MASK;
        vk_code = system_map->vk_code[scancode];
        SET_KEY_UP ( key_states[vk_code] );
        screen_puts("Key released: '");
        if (vk_code) {
            if ( vk_ascii[vk_code] )
                screen_putc(vk_ascii[vk_code]);
            else {
                screen_put_int(vk_code);
                screen_puts(" [NO ASCII]");
            }
        } else {
                screen_put_int(scancode);
                screen_puts(" [NOT TRANSLATED]");
        }
        screen_puts("'\n");
    } else if ( scancode == KEY_NEED_NEXT_KEY ) {
        /* Not dealing with this yet. Includes right alt, etc.. */
    } else {
        vk_code = system_map->vk_code[scancode];
        SET_KEY_DOWN ( key_states[vk_code] );
        screen_puts("Key pressed : '");
        if (vk_code) {
            if ( vk_ascii[vk_code] )
                screen_putc(vk_ascii[vk_code]);
            else {
                screen_put_int(vk_code);
                screen_puts(" [NO ASCII]");
            }
        } else {
                screen_put_int(scancode);
                screen_puts(" [NOT TRANSLATED]");
        }
        screen_puts("'\n");
    }

}

PRIVATE keyboard_map_t en_US_keymap = {
    "en-US",

    {0} /* To be initialized 0 to 128 later... */
};

void init_keyboard ( void )
{
    int i;
    for ( i = 0; i < 128; i ++ )
        en_US_keymap.vk_code[i] = i;
    
    system_map = &en_US_keymap;
    register_interrupt_handler ( IRQ_1, &keyboard_handler );
}

uint16_t get_key_state(uint16_t scancode) {
    uint16_t flags = 0;
    if ( ALT_PRESSED() )
        flags |= ALT_FLAG;
    
    if ( CTRL_PRESSED() )
        flags |= CTRL_FLAG;
        
    if ( LSHIFT_PRESSED() )
        flags |= SHIFT_FLAG;
        
    if ( CAPS_ON() )
        flags |= CAPS_FLAG;
        
    return flags | key_states[scancode];
}
