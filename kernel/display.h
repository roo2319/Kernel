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


typedef struct {
  int x,y;
} coord_t;


extern void print(char* s);
extern void init_display(uint16_t fb[600][800], coord_t* mouse, coord_t* cursor);
extern void handle_keyboard(uint16_t fb[600][800], coord_t* cursor, coord_t* mouse);
extern void handle_mouse_move(uint16_t fb[600][800], coord_t* mouse);