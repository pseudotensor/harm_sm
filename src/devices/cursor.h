#include "copyright.h"
#define cursor_width 19
#define cursor_height 19
#define cursor_x_hot 9
#define cursor_y_hot 9
static short cursor_bits[] = {
   0x0200, 0x0000, 0x0200, 0x0000,
   0x0200, 0x0000, 0x0200, 0x0000,
   0x0200, 0x0000, 0x0200, 0x0000,
   0x0200, 0x0000, 0x0200, 0x0000,
   0x0500, 0x0000, 0xf8ff, 0x0007,
   0x0500, 0x0000, 0x0200, 0x0000,
   0x0200, 0x0000, 0x0200, 0x0000,
   0x0200, 0x0000, 0x0200, 0x0000,
   0x0200, 0x0000, 0x0200, 0x0000,
   0x0200, 0x0000};
#define cursor_mask_width 19
#define cursor_mask_height 19
#define cursor_mask_x_hot 9
#define cursor_mask_y_hot 9
static short cursor_mask_bits[] = {
   0x0700, 0x0000, 0x0700, 0x0000,
   0x0700, 0x0000, 0x0700, 0x0000,
   0x0700, 0x0000, 0x0700, 0x0000,
   0x0700, 0x0000, 0x0700, 0x0000,
   0xfdff, 0x0007, 0xf8ff, 0x0007,
   0xfdff, 0x0007, 0x0700, 0x0000,
   0x0700, 0x0000, 0x0700, 0x0000,
   0x0700, 0x0000, 0x0700, 0x0000,
   0x0700, 0x0000, 0x0700, 0x0000,
   0x0700, 0x0000};
