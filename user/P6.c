#include "P6.h"

//Create text on display in "random" colours
void main_P6() {
  for (int i = 0; i<20; i++){ 
    display_put("I love concurrent.\n",20,get_random()%0x10000);
    yield();
  }


  exit( EXIT_SUCCESS );
}
