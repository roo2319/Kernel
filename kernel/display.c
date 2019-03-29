#include "display.h"

int mouse_packet[3];int mouse_packet_no = 0; uint16_t undermouse[5][5]; uint16_t undercursor[10]; bool released = false;

int const Font[31][7] = { 
   {0x04, 0x0a, 0x11, 0x11, 0x1f, 0x11, 0x11},   //A
   {0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e},   //B
   {0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e},   //C
   {0x1e, 0x09, 0x09, 0x09, 0x09, 0x09, 0x1e},   //D
   {0x1f, 0x10, 0x10, 0x1c, 0x10, 0x10, 0x1f},   //E
   {0x1f, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10},   //F
   {0x0e, 0x11, 0x10, 0x10, 0x13, 0x11, 0x0f},   //G
   {0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},   //H
   {0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e},   //I
   {0x1f, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c},   //J
   {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},   //K
   {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f},   //L
   {0x11, 0x1b, 0x15, 0x11, 0x11, 0x11, 0x11},   //M
   {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11},   //N
   {0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},   //O
   {0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10},   //P
   {0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d},   //Q
   {0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11},   //R
   {0x0e, 0x11, 0x10, 0x0e, 0x01, 0x11, 0x0e},   //S
   {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},   //T
   {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},   //U
   {0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04},   //V
   {0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11},   //W
   {0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11},   //X
   {0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04},   //Y
   {0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f},   //Z
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c},   //.
   {0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08},   //,
   {0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00},   // ' 
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},    //space
   {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}    //Square character
   }; 

void print(PL011_t* uart, char* s){
  char ind = 0;
  char c = s[0];
  while (c != '\0'){
    PL011_putc(uart,c,true);
    ind++;
    c=s[ind];
  }
}
void remove_cursor(uint16_t fb[600][800], coord_t* cursor){
  for( int i = 0; i<7; i++){
    fb[cursor->y+i][cursor->x] = undercursor[i];
  }
}

void draw_cursor(uint16_t fb[600][800], coord_t* cursor){
  for( int i = 0; i<7; i++){
    undercursor[i] = fb[cursor->y+i][cursor->x];
    fb[cursor->y+i][cursor->x] = GREEN;
  }
}

void move_cursor(uint16_t fb[600][800], coord_t* cursor, int x, int y){
  remove_cursor(fb,cursor);
  cursor->x = x;
  cursor->y = y;
  draw_cursor(fb,cursor);

}

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
  for( int i = 0; i < 5; i++ ) {
    for( int j = 0; j < 5; j++ ) {
      undermouse[ i ][ j ] = 0;
    }
    //Initialised undermouse with cursor
    undermouse[i][0] = GREEN;
    undermouse[i+5][0] = GREEN;
  }
  draw_cursor(fb,cursor);

  return;
}

void draw_rectangle(uint16_t fb[600][800],int x, int y, int xlen, int ylen, int colour){
  for (int i = 0; i<xlen;i++){
    for (int j = 0;j<ylen;j++){
      fb[y+j][x+i] = colour;
    }
  }
}

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


void remove_char(uint16_t fb[600][800], coord_t* cursor){
    for( int i = 0; i < 7; i++ ) {
              for( int j = 0; j < 8; j++ ) {
                  fb[ cursor->y + i ][cursor->x + j ] = 0;
              }
          }
          draw_cursor(fb,cursor);
}

void handle_newline(uint16_t fb[600][800], coord_t* cursor){
  if (cursor->y + 14 < 600){
    remove_cursor(fb,cursor);
    cursor->x = 0;
    cursor->y+=14;
    draw_cursor(fb,cursor);
  }

}


void handle_scancode(uint16_t fb[600][800], coord_t* cursor, coord_t* mouse, uint8_t x){
  if (!released){
    switch (x)
    {
     //Q,example character
      case 0x15:
        draw_char(fb,cursor,'Q'-'A',GREEN);
        break;
      //W
      case 0x1D:
        draw_char(fb,cursor,'W'-'A',GREEN);
        break;

      //E
      case 0x24:
        draw_char(fb,cursor,'E'-'A',GREEN);
        break;

      //R
      case 0x2D:
        draw_char(fb,cursor,'R'-'A',GREEN);
        break;

      //T
      case 0x2C:
        draw_char(fb,cursor,'T'-'A',GREEN);
        break;

      //Y
      case 0x35:
        draw_char(fb,cursor,'Y'-'A',GREEN);
        break;
      
      //U
      case 0x3C:
        draw_char(fb,cursor,'U'-'A',GREEN);
        break;

      //I
      case 0x43:
        draw_char(fb,cursor,'I'-'A',GREEN);
        break;
      
      //O
      case 0x44:
        draw_char(fb,cursor,'O'-'A',GREEN);
        break;

      //P
      case 0x4D:
        draw_char(fb,cursor,'P'-'A',GREEN);
        break;
      
      //A
      case 0x1C:
        draw_char(fb,cursor,'A'-'A',GREEN);
        break;

      //S
      case 0x1B:
        draw_char(fb,cursor,'S'-'A',GREEN);
        break;

      //D
      case 0x23:
        draw_char(fb,cursor,'D'-'A',GREEN);
        break;

      //F
      case 0x2B:
        draw_char(fb,cursor,'F'-'A',GREEN);
        break;

      //G
      case 0x34:
        draw_char(fb,cursor,'G'-'A',GREEN);
        break;

      //H
      case 0x33:
        draw_char(fb,cursor,'H'-'A',GREEN);
        break;

      //J
      case 0x3B:
        draw_char(fb,cursor,'J'-'A',GREEN);
        break;

      //K
      case 0x42:
        draw_char(fb,cursor,'K'-'A',GREEN);
        break;

      //L
      case 0x4B:
        draw_char(fb,cursor,'L'-'A',GREEN);
        break;

      //Z
      case 0x1A:
        draw_char(fb,cursor,'Z'-'A',GREEN);
        break;

      //X
      case 0x22:
        draw_char(fb,cursor,'X'-'A',GREEN);
        break;

      //C
      case 0x21:
        draw_char(fb,cursor,'C'-'A',GREEN);
        break;

      //V
      case 0x2A:
        draw_char(fb,cursor,'V'-'A',GREEN);
        break;
      
      //B
      case 0x32:
        draw_char(fb,cursor,'B'-'A',GREEN);
        break;

      //N
      case 0x31:
        draw_char(fb,cursor,'N'-'A',GREEN);
        break;
      
      //M
      case 0x3A:
        draw_char(fb,cursor,'M'-'A',GREEN);
        break;

      //.
      case 0x49:
        draw_char(fb,cursor,26,GREEN);
        break;

      //,
      case 0x41:
        draw_char(fb,cursor,27,GREEN);
        break;

      //'
      case 0x52:
        draw_char(fb,cursor,28,GREEN);
        break;
      
      //Spacebar
      case 0x29:
        draw_char(fb,cursor,29,GREEN);
        break;
      
      case 0x1E:
        draw_char(fb,cursor,30,0x1234);
        break;
        
      //Backspace
      case 0x66:
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
        break;


        

      //Enter
      case 0x5A:
        handle_newline(fb, cursor);
        break;
      //Signals button released
      case 0xF0:
        released = true;
        break;
      //1 on numpad, signals clear screen
      case 0x16:
        init_display(fb,mouse,cursor);
        break;
    
      default:
        print(UART0,"Character not supported!"); 
        break;
    }
  }
  else
  {
    released = false;
  } 
}

void handle_keyboard(uint16_t fb[600][800], coord_t* cursor, coord_t* mouse){
    uint8_t x = PL050_getc( PS20 ); 
    handle_scancode(fb, cursor, mouse, x);
}

void handle_mouse_move(uint16_t fb[600][800], coord_t* mouse){
    mouse_packet[mouse_packet_no] = PL050_getc( PS21 );
    mouse_packet_no++;
    if (mouse_packet_no == 3){
      for (int i = 0; i<5 && mouse->y + i < 600; i++){
        for (int j = 0; j<5 && mouse->x + j < 800; j++){
          if (fb[(mouse->y+i)][(mouse->x+j)] == WHITE){
            fb[(mouse->y+i)][(mouse->x+j)] = undermouse[i][j];
          }
        }
      }
      //Setup right wall
      if ((mouse->x + mouse_packet[1] - ((mouse_packet[0] << 4) & 0x100)) >= 800){
        mouse->x = 799;
      }
      //Setup left wall
      else if((mouse->x + mouse_packet[1] - ((mouse_packet[0] << 4) & 0x100)) < 0){
        mouse->x = 0;
      }
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
      else{
        mouse->y =  (mouse->y - (mouse_packet[2] - ((mouse_packet[0] << 3) & 0x100))+600)%600;
      }
      mouse_packet_no = 0;
      for (int i = 0; i<5 && mouse->y + i < 600; i++){
        for (int j = 0; j<5 && mouse->x + j < 800; j++){
          undermouse[i][j] = fb[(mouse->y+i)][(mouse->x+j)]; 
          fb[(mouse->y+i)][(mouse->x+j)] = WHITE;
        }
      }
    }
}