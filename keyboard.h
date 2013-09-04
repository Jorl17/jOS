#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdinc.h>
/*
 * This is the first iteration of our very simple keyboard model.
 *
 * The OS, as of the time being, has several keyboard maps.
 * A keyboard map, defined in the structure keyboard_map_t,
 * has a name (for instance, pt-PT, en-US, pt-pT no dead keys, etc),
 * as well as a table of vks.
 *
 * What the table of vks does is convert scancodes (received from the keyboard)
 * onto VK (Virtual Key) values. If we receive scancode 5, we access
 * vk_code[5] to retrieve the respective VK.
 *
 * For instance, if a keyboard_map intends to map scancode 10 to the letter A,
 * one only needs to make sure tht vk_code[10] = VK_A.
 *
 * The VKs were setup in such a way that their numbers match the scancodes
 * of the en-US layout when in "Scan Code Set 1". This is useful because
 * one can then build the en-US vk_code array by simply mapping each
 * index to itself. That is, vk_code[i] = i, in the case of the en-US layout.
 *
 * Other layouts will have to map vk_codes differently.
 *
 * In the future, an API for loading keyboard maps and initializing them
 * might be used. Right now, it's all done in  init_keyboard()
 *
 * We also provide a get_key_state function which returns a 16-bit
 * value that gives us the state of the requested key (given by a VK),
 * as well as all the modifiers (ALT, CTRL, SHIFT, CAPS). Thus, it is
 * a simple bitfield.
 *
 * For instance, to check if the letter A is pressed, one must only do:
 *
 * if ( IS_KEY_DOWN(get_key_state(VK_A) ) )
 *
 * FIXME: Soon we should implement some kind of callback mechanism
 * for keypresses, etc
 */


#define KEYB_DATA_PORT    0x60
#define KEYB_STATUS_PORT  0x63
#define KEYB_COMMAND_PORT KEYB_STATUS_PORT

#define KEY_RELEASED_MASK 0x80
#define KEY_NEED_NEXT_KEY 0xE0

#define KEY_DOWN_STATE    1
#define ALT_FLAG          512
#define CTRL_FLAG         1024
#define SHIFT_FLAG        2048
#define CAPS_FLAG         4096
#define IS_KEY_DOWN(x) (((x) & KEY_DOWN_STATE) == KEY_DOWN_STATE )

/* 128 keyboard_map being used */
typedef struct _keyboard_map_t {
    const char name[32]; /* pt-PT, en-US, etc */
    uint16_t   vk_code[128];
} keyboard_map_t;

void init_keyboard ( void );
uint16_t get_key_state ( uint16_t scancode );


/* #define VK_ FIXME: WHAT'S HERE? */
#define VK_27 1 /* what 's 27? */
#define VK_1 2
#define VK_2 3
#define VK_3 4
#define VK_4 5
#define VK_5 6
#define VK_6 7
#define VK_7 8
#define VK_8 9
#define VK_9 10
#define VK_0 11
#define VK_MINUS 12
#define VK_EQUAL 13
#define VK_BACKSPACE 14
#define VK_TAB 15
#define VK_Q 16
#define VK_W 17
#define VK_E 18
#define VK_R 19
#define VK_T 20
#define VK_Y 21
#define VK_U 22
#define VK_I 23
#define VK_O 24
#define VK_P 25
#define VK_LEFTPARSTRAIGHT 26
#define VK_RIGHTPARSTRAIGHT 27
#define VK_ENTER 28
#define VK_CTRL 29
#define VK_A 30
#define VK_S 31
#define VK_D 32
#define VK_F 33
#define VK_G 34
#define VK_H 35
#define VK_J 36
#define VK_K 37
#define VK_L 38
#define VK_SEMICOLON 39
#define VK_UPPERCOMMA 40
#define VK_APOSTROPHE 41
#define VK_LSHIFT 42
#define VK_BACKSLASH 43
#define VK_Z 44
#define VK_X 45
#define VK_C 46
#define VK_V 47
#define VK_B 48
#define VK_N 49
#define VK_M 50
#define VK_COMMA 51
#define VK_DOT 52
#define VK_FORWARDSLASH 53
#define VK_RSHIFT 54
#define VK_ALT 55
#define VK_SPAE 56
#define VK_CAPS 57
#define VK_F1 58
#define VK_F2 59
#define VK_F3 60
#define VK_F4 61
#define VK_F5 62
#define VK_F6 63
#define VK_F7 64
#define VK_F8 65
#define VK_F9 66
#define VK_F10 67
#define VK_NUMLOCK 68
#define VK_SCROLLLOCK 69
#define VK_HOME 70
#define VK_UP 71
#define VK_PAGEUP 72
#define VK_KPMINUS 73
#define VK_LEFT 74
/* #define VK_ FIXME: WHAT'S HERE? */
#define VK_RIGHT 76
#define VK_KPPLUS 77
#define VK_END 78
#define VK_DOWN 79
#define VK_PAGEDOWN 80
#define VK_INSERT 81
#define VK_DELETE 82
/* #define VK_ FIXME: WHAT'S HERE? */
/* #define VK_ FIXME: WHAT'S HERE? */
/* #define VK_ FIXME: WHAT'S HERE? */
#define VK_F11 86
#define VK_F12 87

/* These keys do not exist as direct mappings in the US keyboard,
 * they have no scancode (or it's a more-than-one-byte scancode)
 * and are usually the keys that are triggered by shift, ctrl or alt, etc
 */
#define VK_PLUS 256
#define LAST_VK VK_PLUS

#endif
