#include <conway.h>

#define IMHT 100
#define IMWD 100

char grid[IMHT][IMWD]={{}};
char grid_ghost[IMHT+2][IMWD]={{}};

//Mutex used for access to the conway grid
char access = 1;

/*
program that will take in a rectangular slice of conways and process it
assuming the top and bottom rows are 'ghost rows' and the screen loops
*/

//returns x mod y, supporting values from -y to +MAX_INT
int mod(int x, int y){
    return (x+y)%y;
}


//Draws the state of conways to the display. 
//The current size is 8px * 6 px  cells, 100*100 for fb
void update_display(){
    for (int i = 0; i < IMHT; i++){
        for (int j = 0; j < IMWD; j++){
            draw_rect(i*8,j*6,8,6,0x7FFF*grid[j][i]);
        }
    }
}

//ARRAY is specified as [row][column]
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
            }
            else
            {
                out[i-1][j] = 1;
            }
        }
    }
    update_display();
}


//Sets all cells to zero then updates display
//Used by IRQ on keypress "1"
void conway_reset(){
    sem_wait(&access);
    for (int i = 0; i<IMHT; i++){
        for (int j = 0; j<IMWD;j++){
            grid[i][j] = 0;
        }
    }
    update_display();
    sem_post(&access);
}

//Create a glider in the top right
void conway_glider(){
    sem_wait(&access);
    grid[0][1] = 1;
    grid[1][2] = 1;
    grid[2][0] = 1;
    grid[2][1] = 1;
    grid[2][2] = 1;
    update_display();
    sem_post(&access);
    exit(EXIT_SUCCESS);
}


//Create a line in row 50, except for the leftmost and rightmost cell
void conway_line(){
    sem_wait(&access);
    for (int i = 1; i<IMWD-1;i++){
        grid[50][i] = 1;
    }
    update_display();
    sem_post(&access);
    exit(EXIT_SUCCESS);
}

//Creates a glider at the location of mouse
//An example of using a syscall to get mouse position
void conway_mouse_glider(){
    
    char col = get_mouse_x()/8;
    char row = get_mouse_y()/6;
    sem_wait(&access);
    grid[  row  ][(col+1)] = 1;
    grid[(row+1)][(col+2)] = 1;
    grid[(row+2)][(col+0)] = 1;
    grid[(row+2)][(col+1)] = 1;
    grid[(row+2)][(col+2)] = 1;
    update_display();
    sem_post(&access);
    exit(EXIT_SUCCESS);
}

//Called by the display in order to handle mouse input
//Writes to a particular cell
void conway_from_mouse(int x, int y, bool draw){
    char col = x/8;
    char row = y/6;
    grid[row][col] = (int) draw;
    update_display();
}

//Forever iterate conway
void conway_start(){

    while(true){
        sem_wait(&access);
        memcpy(grid_ghost,grid[IMHT-1],sizeof(grid[IMHT-1]));
        memcpy(grid_ghost[1],grid,sizeof(grid));
        memcpy(grid_ghost[IMHT+1],grid[0],sizeof(grid[0]));
        conway(grid,grid_ghost);
        sem_post(&access);
        yield();
    }
}