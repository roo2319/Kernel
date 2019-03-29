#include "P7.h"

int colours[3] = {0x001F,0x03E0,0x7C00};
int rn = 239358332;

void main_P7() {
  while( 1 ) {
    for (int i = 0; i<3; i++){
      rn = LCG(rn); 
      display_put("I am rUaIrI\n",13,rn%0x10000);

    }

    yield();
  }

  exit( EXIT_SUCCESS );
}
