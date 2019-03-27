#include "display.h"

int mouse_packet[3];int mouse_packet_no = 0; uint16_t undermouse[5][5]; uint16_t undercursor[10]; bool released = false;

void print(char* s){
  char ind = 0;
  char c = s[0];
  while (c != '\0'){
    PL011_putc(UART0,c,true);
    ind++;
    c=s[ind];
  }
}

void remove_cursor(uint16_t fb[600][800], coord_t* cursor){
    for( int i = 0; i<10; i++){
            fb[cursor->y+i][cursor->x] = undercursor[i];
    }
}

void draw_char(uint16_t fb[600][800], coord_t* cursor){
    for( int i = 0; i < 10; i++ ) {
                for( int j = 0; j < 5; j++ ) {
                    fb[ cursor->y + i ][cursor->x + j ] = 0x03E0;
                }
            }
            cursor->x+=10;
            for( int i = 0; i<10; i++){
                undercursor[i] = fb[cursor->y+i][cursor->x];
                fb[cursor->y+i][cursor->x] = 0x03E0;
            }
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
    undermouse[i][0] = 0x03E0;
    undermouse[i+5][0] = 0x03E0;
  }
  for( int i = 0; i<10; i++){
      undercursor[i] = 0;
      fb[i][0] = 0x03E0;
  }

  return;
}

void handle_scancode(uint16_t fb[600][800], coord_t* cursor, coord_t* mouse, uint8_t x){
  if (!released){
    switch (x)
    {
     //Q,example character
      case 0x15:
        remove_cursor(fb,cursor);
        draw_char(fb,cursor);


        break;


      case 0x5A:
        remove_cursor(fb,cursor);
        cursor->x = 0;
        cursor->y+=20;
        for( int i = 0; i<10; i++){
            undercursor[i] = fb[cursor->y+i][cursor->x];
            fb[cursor->y+i][cursor->x] = 0x03E0;
        }

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
        print("Character not supported!"); 
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
          fb[(mouse->y+i)][(mouse->x+j)] = undermouse[i][j];
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
          fb[(mouse->y+i)][(mouse->x+j)] = 0x7FFF;
        }
      }
    }
}