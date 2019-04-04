#include "display.h"

int mouse_packet[3];int mouse_packet_no = 0; uint16_t undermouse[7][8]; uint16_t undercursor[10]; bool released = false;
bool has_mouse_changed; 


//0 for lab machine, 1 for my laptop.
int scancode_no = 1;

extern void conway_from_mouse(int x, int y, bool draw);
extern void conway_reset();

//Design of the mouse, expressed as pixels. Easy to change.
int mouse_design[7] ={
  0b11111000,
  0b11111000,
  0b11100000,
  0b11010000,
  0b11001000,
  0b00000100,
  0b00000010,
};

int const scancode[34][2] = {
  {0x10, 0x15}, //Q
  {0x11, 0x1D}, //W
  {0x12, 0x24}, //E
  {0x13, 0x2D}, //R
  {0x14, 0x2C}, //T
  {0x15, 0x35}, //Y
  {0x16, 0x3C}, //U
  {0x17, 0x43}, //I
  {0x18, 0x44}, //O
  {0x19, 0x4D}, //P
  {0x1E, 0x1C}, //A
  {0x1F, 0x1B}, //S
  {0x20, 0x23}, //D
  {0x21, 0x2B}, //F
  {0x22, 0x34}, //G
  {0x23, 0x33}, //H
  {0x24, 0x3B}, //J
  {0x25, 0x42}, //K
  {0x26, 0x4B}, //L
  {0x2C, 0x1A}, //Z
  {0x2D, 0x22}, //X
  {0x2E, 0x21}, //C
  {0x2F, 0x2A}, //V
  {0x30, 0x32}, //B
  {0x31, 0x31}, //N
  {0x32, 0x3A}, //M
  {0x33, 0x41}, //,
  {0x34, 0x49}, //.
  {0x28, 0x52}, //'
  {0x39, 0x29}, // Space
  {0x02, 0x16}, // 1
  {0x03, 0x1E}, // 2
  {0x0E, 0x66}, // Backspace
  {0x1C, 0x5A}, //Enter
  
};

//Bit packed font for rendering characters
int const Font[31][7] = { 
   {0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d},   //Q
   {0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11},   //W
   {0x1f, 0x10, 0x10, 0x1c, 0x10, 0x10, 0x1f},   //E
   {0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11},   //R
   {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},   //T
   {0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04},   //Y
   {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},   //U
   {0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e},   //I
   {0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},   //O
   {0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10},   //P
   {0x04, 0x0a, 0x11, 0x11, 0x1f, 0x11, 0x11},   //A
   {0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e},   //S
   {0x1e, 0x09, 0x09, 0x09, 0x09, 0x09, 0x1e},   //D
   {0x1f, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10},   //F
   {0x0e, 0x11, 0x10, 0x10, 0x13, 0x11, 0x0f},   //G
   {0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},   //H
   {0x1f, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c},   //J
   {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},   //K
   {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f},   //L
   {0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f},   //Z
   {0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11},   //X
   {0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e},   //C
   {0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04},   //V
   {0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e},   //B
   {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11},   //N
   {0x11, 0x1b, 0x15, 0x11, 0x11, 0x11, 0x11},   //M
   {0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08},   //,
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c},   //.
   {0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00},   // ' 
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},    //space
   {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}    //Square character
   }; 


//Replace the cursor with the undercursor array
void remove_cursor(uint16_t fb[600][800], coord_t* cursor){
  for( int i = 0; i<7; i++){
    fb[cursor->y+i][cursor->x] = undercursor[i];
  }
}

//Draw cursor and update undercursor array
void draw_cursor(uint16_t fb[600][800], coord_t* cursor){
  for( int i = 0; i<7; i++){
    undercursor[i] = fb[cursor->y+i][cursor->x];
    fb[cursor->y+i][cursor->x] = GREEN;
  }
}

//Self explanatory
void move_cursor(uint16_t fb[600][800], coord_t* cursor, int x, int y){
  remove_cursor(fb,cursor);
  cursor->x = x;
  cursor->y = y;
  draw_cursor(fb,cursor);

}

//Move mouse and cursor to 0,0. Set screen to black. Reset conway.
void init_display(uint16_t fb[600][800],coord_t* mouse,coord_t* cursor){

  mouse->x = 0;
  mouse->y = 0;
  cursor->x = 0;
  cursor->y = 0;

  for( int i = 0; i < 600; i++ ) {
    for( int j = 0; j < 800; j++ ) {
        fb[ i ][ j ] = 0;
    }
  }
  draw_cursor(fb,cursor);
  for( int i = 0; i < 7; i++ ) {
    for( int j = 0; j < 8; j++ ) {
      undermouse[ i ][ j ] = fb[i][j];
    }
  }
  conway_reset();
  
  return;
}

//Draw rectangle at x,y of dimensions xlen, ylen
void draw_rectangle(uint16_t fb[600][800],int x, int y, int xlen, int ylen, int colour){
  for (int i = 0; i<xlen;i++){
    for (int j = 0;j<ylen;j++){
      fb[y+j][x+i] = colour;
    }
  }
}

//Write a character to the cursor location and then move it.
//Well defined for A-Z and . , '
void draw_char(uint16_t fb[600][800], coord_t* cursor,char character,int colour){
    remove_cursor(fb,cursor);
    //Draw character
    for( int i = 0; i < 7; i++ ) {
                for( int j = 0; j < 8; j++ ) {
                    fb[ cursor->y + i ][cursor->x + j ] = ((Font[character][i] & (1<<(7-j)))>>(7-j)) * colour;
                }
            }
    //Move cursor, moving down a line if it is at the end
    if (cursor->x==784){
      if (cursor->y+14<600){
        cursor->y+=14;
        cursor->x=0;
      }
    }
    else{
      cursor->x+=16;
    }
    draw_cursor(fb,cursor);
}

//Remove the mouse from the display by replacing with buffer
void remove_mouse(uint16_t fb[600][800],coord_t* mouse){
  for (int i = 0; i<7 && mouse->y + i < 600; i++){
          for (int j = 0; j<8 && mouse->x + j < 800; j++){
            fb[(mouse->y+i)][(mouse->x+j)] = undermouse[i][j];
          }
        }
}

//Redner the mouse to the screen, treating 1 as white and 2 as transparent
//Although line 3 looks horrible, it is just bit unpacking and a ternary operator
void draw_mouse(uint16_t fb[600][800], coord_t* mouse){
  for( int i = 0; i < 7 && mouse->y + i < 600; i++ ) {
      for( int j = 0; j < 8 && mouse->x + j < 800; j++ ) {
          fb[ mouse->y + i ][mouse->x + j ] = ((mouse_design[i] & (1<<(7-j)))>>(7-j))==1?WHITE:fb[mouse->y +i][mouse->x+j];
      }
  }
}

//Works as backspace, replaces 7x8 region behind cursor with black
void remove_char(uint16_t fb[600][800], coord_t* cursor){
    for( int i = 0; i < 7; i++ ) {
              for( int j = 0; j < 8; j++ ) {
                  fb[ cursor->y + i ][cursor->x + j ] = 0;
              }
          }
          draw_cursor(fb,cursor);
}

//Moves cursor down, unless on bottom line
void handle_newline(uint16_t fb[600][800], coord_t* cursor){
  if (cursor->y + 14 < 600){
    remove_cursor(fb,cursor);
    cursor->x = 0;
    cursor->y+=14;
    draw_cursor(fb,cursor);
  }

}

/*Big case statement to handle scan codes. 
*Key functionality is:
* For A-Z and .,' print to display
* For space print a black box and move cursor
* For backspace, remove the previous character, handling newlines
* For enter go down a line
* For 1 reset the screen
* For 2 print a test character, to test for screen issues
*/
void handle_scancode(uint16_t fb[600][800], coord_t* cursor, coord_t* mouse, uint8_t x){
  if (!released){
      //Signals button released
      if (scancode_no == 1){
        if (x == 0xF0){
          released = true;
          return;
        }
      }

      //Spacebar
      if (x == scancode[29][scancode_no]){
        draw_char(fb,cursor,29,GREEN);
        return;
      }
      
      //1, signals clear screen
      else if (x == scancode[30][scancode_no]){
        init_display(fb,mouse,cursor);
        return;
      }

      //2
      else if (x == scancode[31][scancode_no]){
        draw_char(fb,cursor,30,0x1234);
        return;
      }
        
      //Backspace
      else if (x == scancode[32][scancode_no]){
        remove_cursor(fb,cursor);
        if (cursor->x == 0){
          if (cursor->y != 0){
            cursor->y-=14;
            cursor->x=784;
          }
        }
        else{
          cursor->x-=16;
        }
        remove_char(fb,cursor);
        return;
      }

      //Enter
      else if (x == scancode[33][scancode_no]){
        handle_newline(fb, cursor);
        return;
      }

      else{
        for (int i = 0; i<29; i++){
          if (scancode[i][scancode_no] == x){
            draw_char(fb,cursor,i,GREEN);
            return;
          }
        }
      }
    }
  else{
    released = false;
  } 
}

//Function exposed to hilevel.c
void handle_keyboard(uint16_t fb[600][800], coord_t* cursor, coord_t* mouse){
    uint8_t x = PL050_getc( PS20 ); 
    handle_scancode(fb, cursor, mouse, x);
}

//Function exposed to hilevel.c
void handle_mouse_move(uint16_t fb[600][800], coord_t* mouse){
    mouse_packet[mouse_packet_no] = PL050_getc( PS21 );
    mouse_packet_no++;

    //Only parse once 3 packets have been recieved, as they come in 3's
    if (mouse_packet_no == 3){

      //To decide if we can replace with undermouse buffer we must determine if the mouse has changed.
      has_mouse_changed = false;
      for (int i = 0; i<7 && mouse->y + i < 600; i++){
        for (int j = 0; j<8 && mouse->x + j < 800; j++){
          if ((((mouse_design[i] & (1<<(7-j)))>>(7-j)) == WHITE) && (fb[(mouse->y+i)][(mouse->x+j)] != ((mouse_design[i] & (1<<(7-j)))>>(7-j)) * WHITE)){
            has_mouse_changed = true;
          }
        }
      }

      //If the mouse has not changed, then we can safely replace with the under mouse buffer
      if (!has_mouse_changed){
        remove_mouse(fb,mouse);
        
      }
      //Setup right wall
      if ((mouse->x + mouse_packet[1] - ((mouse_packet[0] << 4) & 0x100)) >= 800){
        mouse->x = 799;
      }
      //Setup left wall
      else if((mouse->x + mouse_packet[1] - ((mouse_packet[0] << 4) & 0x100)) < 0){
        mouse->x = 0;
      }
      //Otherwise parse as normal
      else{
        mouse->x =  (mouse->x + mouse_packet[1] - ((mouse_packet[0] << 4) & 0x100));
      }

      //Setup bottom wall
      if ((mouse->y - (mouse_packet[2] - ((mouse_packet[0] << 3) & 0x100))) >= 600){
        mouse->y = 599;
      }
      //Setup top wall
      else if((mouse->y - (mouse_packet[2] - ((mouse_packet[0] << 3) & 0x100))) < 0){
        mouse->y = 0;
      }
      //Same here
      else{
        mouse->y =  (mouse->y - (mouse_packet[2] - ((mouse_packet[0] << 3) & 0x100))+600)%600;
      }

      //We will check for clicks and feed the data to conway
      
      //LMB
      if ((mouse_packet[0] & 1) == 1){
        conway_from_mouse(mouse->x,mouse->y,true);
      }
      //RMB
      else if ((mouse_packet[0] & (1<<1)) == 2){
        conway_from_mouse(mouse->x,mouse->y,false);
      }

      //0 the count, refill undermouse, and redraw the mouse
      mouse_packet_no = 0;
      for (int i = 0; i<7 && mouse->y + i < 600; i++){
        for (int j = 0; j<8 && mouse->x + j < 800; j++){
          undermouse[i][j] = fb[(mouse->y+i)][(mouse->x+j)]; 
        }
      }
      draw_mouse(fb,mouse);
    }
}