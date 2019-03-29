#include <P8.h>

#define IMHT 100
#define IMWD 100

char grid[IMHT][IMWD]={{}};
char grid_ghost[IMHT+2][IMWD]={{}};

/*
program that will take in a rectangular slice of conways and process it
assuming the top and bottom rows are 'ghost rows' and the screen loops
*/

//returns x mod y
int mod(int x, int y){
    return (x+y)%y;
}


//ARRAY is specified as [y][x]
//Takes in conway with ghost rows and returns without
void conway(char out[IMHT][IMWD] , char in[IMHT+2][IMWD]){
    for (int i = 1; i < IMHT+1; i++)
    {
        for (int j = 0; j < IMWD; j++)
        {
            char neighbours = in[i-1][mod(j-1,IMWD)] + in[i-1][j] + in[i-1][mod(j+1,IMWD)] 
                            + in[i][mod(j-1,IMWD)]                + in[i][mod(j+1,IMWD)]
                            + in[i+1][mod(j-1,IMWD)] + in[i+1][j] + in[i+1][mod(j+1,IMWD)];
            if ((in[i][j] == 1 && (neighbours < 2 || neighbours > 3)) || (in[i][j] == 0 && neighbours != 3))
            {
                out[i-1][j] = 0;
                draw_rect(j*8,(i-1)*6,8,6,0);
            }
            else
            {
                out[i-1][j] = 1;
                draw_rect(j*8,(i-1)*6,8,6,0x7FFF);
            }
        }
    }
}

void main_P8(){
    //Access it in row column format
    for (int i = 1; i<IMWD-1;i++){
        grid[50][i] = 1;
    }
    while(true){
        memcpy(grid_ghost,grid[IMHT-1],sizeof(grid[IMHT-1]));
        memcpy(grid_ghost[1],grid,sizeof(grid));
        memcpy(grid_ghost[IMHT+1],grid[0],sizeof(grid[0]));
        conway(grid,grid_ghost);
        yield();
    }
}