#ifndef __display_H
#define __display_H


#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
// Include functionality relating to the platform.

#include "PL011.h"
#include "SP804.h"
#include "PL050.h"
#include "PL111.h"
#include "GIC.h"
#include "SYS.h"

#define BLACK 0
#define WHITE 0x7FFF
#define RED   0x001F
#define GREEN 0x03E0
#define BLUE  0x7C00


typedef struct {
  int x,y;
} coord_t;


extern void init_display(uint16_t fb[600][800], coord_t* mouse, coord_t* cursor);
extern void draw_rectangle(uint16_t fb[600][800],int x, int y, int xlen, int ylen, int colour);
extern void draw_char(uint16_t fb[600][800],coord_t* cursor, char character, int colour);
extern void handle_newline(uint16_t fb[600][800],coord_t* cursor); 
extern void handle_keyboard(uint16_t fb[600][800], coord_t* cursor, coord_t* mouse);
extern void handle_mouse_move(uint16_t fb[600][800], coord_t* mouse);
#endif