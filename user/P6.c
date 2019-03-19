#include "P6.h"

int pids[16]; 
int forks[16];
int random[16] = {2,3,1,2,5,3,2,3,6,1,4,2,4,1,2,4};
int me = 1;

void pickup(int left,int right){
  sem_wait(&me);
  sem_wait(&(forks[left]));
  sem_wait(&(forks[right]));
  sem_post(&me);
}

void putdown(int left,int right){
  sem_post(&forks[left]);
  sem_post(&forks[right]);
}
//To eat, The philosopher with id needs fork id and fork id + 1
void child(int id){
  char phil[2];
  int randomnumber = id;
  itoa(phil,id);
  int left = id;
  int right= (id+1)%16;
  while(true){
    
    write(STDOUT_FILENO,"Philosopher ",12);
    write(STDOUT_FILENO,phil,2);
    write(STDOUT_FILENO," thinks\n",8);
    randomnumber = LCG(randomnumber);
    sleep(LCG(randomnumber));
    write(STDOUT_FILENO,"Philosopher ",12);
    write(STDOUT_FILENO,phil,2);
    write(STDOUT_FILENO," is hungry\n",12);
    pickup(left,right);
    write(STDOUT_FILENO,"---------\n",10);
    write(STDOUT_FILENO,"Philosopher ",12);
    write(STDOUT_FILENO,phil,2);
    write(STDOUT_FILENO," eats\n",6);
    write(STDOUT_FILENO,"---------\n",10);
    randomnumber = LCG(randomnumber);
    sleep(randomnumber);
    putdown(left,right);
    write(STDOUT_FILENO,"Philosopher ",12);
    write(STDOUT_FILENO,phil,2);
    write(STDOUT_FILENO," puts down their forks\n",23);
  }
}



void main_P6(){
  for (int i = 0; i < 16; i++){
    int isparent = fork();
    if (isparent == 0){
      child(i);
    }
    pids[i] = isparent;
    forks[i] = 1;
  }
    while(true){
      //Print info about philosophers?
      
    }
  exit( EXIT_SUCCESS );
}