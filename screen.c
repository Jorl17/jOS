#include <screen.h>

#define FG_COLOR FG_WHITE
#define BG_COLOR BG_BLACK
#define SCREEN_W 80
#define SCREEN_H 25
#define TAB_WIDTH 8

/* The VGA framebuffer starts at 0xB8000. */
uint16_t* vmem = (uint16_t*) 0xB8000;
uint8_t cursor_x = 0, cursor_y = 0;

#define VIDEO_XY(x,y) vmem[ (x) + (y)*SCREEN_W ]

PRIVATE void update_cursor_pos()
{
  uint16_t pos = cursor_y * SCREEN_W + cursor_x;
  outb(0x3D4, 14);                  /* Tell the VGA board we are setting the high cursor byte. */
  outb(0x3D5, pos >> 8);            /* Send the high cursor byte. */
  outb(0x3D4, 15);                  /* Tell the VGA board we are setting the low cursor byte. */
  outb(0x3D5, pos);                 /* Send the low cursor byte. */
}

/* Scrolls the text on the screen up by one line. */
PRIVATE void scroll()
{ 
  uint16_t blank = ' ' | FG_COLOR | BG_COLOR; 

  /* Row 25 is the end, this means we need to scroll up */
  if(cursor_y >= SCREEN_H)
  {
    int last_line_offset = SCREEN_W*(SCREEN_H-1);
    /* Move the current text chunk that makes up the screen
       back in the buffer by a line */
    int i;
    for (i = 0; i < last_line_offset; i++)
      vmem[i] = vmem[i+SCREEN_W];

    /* The last line should now be blank. Clear it */
    for (i = last_line_offset; i < SCREEN_W*SCREEN_H; i++)
      vmem[i] = blank;

    /* The cursor should now be on the last line. */
    cursor_y = SCREEN_H-1;
  }
}

void screen_putc(char c)
{

  /* Handle a backspace, by moving the cursor back one space */
  if (c == '\r' && cursor_x)
    cursor_x--;

  /* Handle a tab by increasing the cursor's X, but only to a point
     here it is divisible by 8. */
  else if (c == '\t')
    cursor_x = (cursor_x+TAB_WIDTH) & ~(TAB_WIDTH-1);

  /* Handle carriage return */
  else if (c == '\r')
    cursor_x = 0;

  /* Handle newline by moving cursor back to left and increasing the row */
  else if (c == '\n')
  {
    cursor_x = 0;
    cursor_y++;
  }

  /* Handle any other printable character. */
  else if(c >= ' ')
  {
      VIDEO_XY(cursor_x,cursor_y) = c | FG_COLOR | BG_COLOR;
      cursor_x++;
  }

  /* Check if we need to insert a new line because we have reached the end
    of the screen. */
  if (cursor_x >= SCREEN_W)
  {
      cursor_x = 0;
      cursor_y ++;
  }

  /* Scroll the screen if needed. */
  scroll();
  /* Move the hardware cursor. */
  update_cursor_pos();
}
