#ifndef SCREEN_H_INC
#define SCREEN_H_INC
#include <stdinc.h>
#include <types.h>
#include <x86.h>
/* Write character */
void screen_putc(char c);

/* Clear Screen */
void screen_clear();

/* Output a null-terminated ASCII string to the monitor */
void screen_puts(const char* c);

/* Print hex number */
void screen_put_hex(uint32_t n);

/* Print decimal number */
void screen_put_int(int32_t n);
void screen_put_uint(uint32_t n);

/* Set fg and bg colors */
void set_fg_color(uint8_t c);
void set_bg_color(uint8_t c);

#define VGA_BLACK           0
#define VGA_BLUE            1
#define VGA_GREEN           2
#define VGA_CYAN            3
#define VGA_RED             4
#define VGA_MAGENTA         5
#define VGA_BROWN           6
#define VGA_LIGHT_GREY      7
#define VGA_DARK_GREY       8
#define VGA_LIGHT_BLUE      9
#define VGA_LIGHT_GREEN    10
#define VGA_LIGHT_CYAN     11
#define VGA_LIGHT_RED      12
#define VGA_LIGHT_MAGENTA  13
#define VGA_YELLOW         14
#define VGA_WHITE          15

#define FG_FROM_COLOR(x)   ( (x) << 8 )      
#define FG_BLACK           FG_FROM_COLOR(VGA_BLACK)
#define FG_BLUE            FG_FROM_COLOR(VGA_BLUE)
#define FG_GREEN           FG_FROM_COLOR(VGA_GREEN)
#define FG_CYAN            FG_FROM_COLOR(VGA_CYAN)
#define FG_RED             FG_FROM_COLOR(VGA_RED)
#define FG_MAGENTA         FG_FROM_COLOR(VGA_MAGENTA)
#define FG_BROWN           FG_FROM_COLOR(VGA_BROWN)
#define FG_LIGHT_GREY      FG_FROM_COLOR(VGA_LIGHT_GREY)
#define FG_DARK_GREY       FG_FROM_COLOR(VGA_DARK_GREY)
#define FG_LIGHT_BLUE      FG_FROM_COLOR(VGA_LIGHT_BLUE)
#define FG_LIGHT_GREEN     FG_FROM_COLOR(VGA_LIGHT_GREEN)
#define FG_LIGHT_CYAN      FG_FROM_COLOR(VGA_LIGHT_CYAN)
#define FG_LIGHT_RED       FG_FROM_COLOR(VGA_LIGHT_RED)
#define FG_LIGHT_MAGENTA   FG_FROM_COLOR(VGA_LIGHT_MAGENTA)
#define FG_YELLOW          FG_FROM_COLOR(VGA_YELLOW)
#define FG_WHITE           FG_FROM_COLOR(VGA_WHITE)

#define BG_FROM_COLOR(x)   ( (x) << 12 )      
#define BG_BLACK           BG_FROM_COLOR(VGA_BLACK)
#define BG_BLUE            BG_FROM_COLOR(VGA_BLUE)
#define BG_GREEN           BG_FROM_COLOR(VGA_GREEN)
#define BG_CYAN            BG_FROM_COLOR(VGA_CYAN)
#define BG_RED             BG_FROM_COLOR(VGA_RED)
#define BG_MAGENTA         BG_FROM_COLOR(VGA_MAGENTA)
#define BG_BROWN           BG_FROM_COLOR(VGA_BROWN)
#define BG_LIGHT_GREY      BG_FROM_COLOR(VGA_LIGHT_GREY)
#define BG_DARK_GREY       BG_FROM_COLOR(VGA_DARK_GREY)
#define BG_LIGHT_BLUE      BG_FROM_COLOR(VGA_LIGHT_BLUE)
#define BG_LIGHT_GREEN     BG_FROM_COLOR(VGA_LIGHT_GREEN)
#define BG_LIGHT_CYAN      BG_FROM_COLOR(VGA_LIGHT_CYAN)
#define BG_LIGHT_RED       BG_FROM_COLOR(VGA_LIGHT_RED)
#define BG_LIGHT_MAGENTA   BG_FROM_COLOR(VGA_LIGHT_MAGENTA)
#define BG_YELLOW          BG_FROM_COLOR(VGA_YELLOW)
#define BG_WHITE           BG_FROM_COLOR(VGA_WHITE)
#endif
