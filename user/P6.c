#include "P6.h"

int forks[16];
int me = 1;

//Wait for MutualExclusivity then take both forks
void pickup(int left,int right){
  sem_wait(&me);
  sem_wait(&(forks[left]));
  sem_wait(&(forks[right]));
  sem_post(&me);
}

//ME is not required for put down
void putdown(int left,int right){
  sem_post(&forks[left]);
  sem_post(&forks[right]);
}
//To eat, The philosopher with id needs fork id and fork id + 1
void child(int id){
  char phil[2];
  itoa(phil,id);
  int left = id;
  int right= (id+1)%16;
  while(true){
    
    write(STDOUT_FILENO,"Philosopher ",12);
    write(STDOUT_FILENO,phil,2);
    write(STDOUT_FILENO," thinks\n",8);
    //Think for a random amount of time
    sleep((get_random()%3)+2);
    write(STDOUT_FILENO,"Philosopher ",12);
    write(STDOUT_FILENO,phil,2);
    write(STDOUT_FILENO," is hungry\n",12);
    //Attempt to pick up forks when hungry
    pickup(left,right);
    write(STDOUT_FILENO,"---------\n",10);
    write(STDOUT_FILENO,"Philosopher ",12);
    write(STDOUT_FILENO,phil,2);
    write(STDOUT_FILENO," eats\n",6);
    write(STDOUT_FILENO,"---------\n",10);
    //Eat for a random amount of time
    sleep((get_random()%3)+2);
    //Put down their forks (does not require mutex)
    putdown(left,right);
    write(STDOUT_FILENO,"Philosopher ",12);
    write(STDOUT_FILENO,phil,2);
    write(STDOUT_FILENO," puts down their forks\n",23);
    //Loop
  }
}



void main_P6(){
  for (int i = 0; i < 16; i++){
    int isparent = fork();
    if (isparent == 0){
      child(i);
    }
    forks[i] = 1;
  }
  while (true){
    
  }
}