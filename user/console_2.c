/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "console_2.h"


extern void puts( char* x, int n );
extern void gets( char* x, int n );

/* Since we lack a *real* loader (as a result of also lacking a storage
 * medium to store program images), the following function approximates 
 * one: given a program name from the set of programs statically linked
 * into the kernel image, it returns a pointer to the entry point.
 */

extern void main_P3(); 
extern void main_P4(); 
extern void main_P5(); 
extern void main_P6();
extern void main_philosophers();
extern void conway_start();
extern void conway_line();
extern void conway_glider();
extern void conway_reset();
extern void conway_mouse_glider();

void* load_2( char* x ) {
  if     ( 0 == strcmp( x, "P3" ) ) {
    return &main_P3;
  }
  else if( 0 == strcmp( x, "P4" ) ) {
    return &main_P4;
  }
  else if( 0 == strcmp( x, "P5" ) ) {
    return &main_P5;
  }
  else if( 0 == strcmp(x,"P6")){
    return &main_P6;
  }
  else if( 0 == strcmp(x, "philosophers")){
    return &main_philosophers;
  }


  return NULL;
}

//Similar to load, but functions only work for conway
void* conway_load( char* x){
  if ( 0 == strcmp(x, "start")){
    return &conway_start;
  }
  else if ( 0 == strcmp(x, "line")){
    return &conway_line;
  }
  else if ( 0 == strcmp(x, "glider")){
    return &conway_glider;
  }
  else if ( 0 == strcmp(x, "mouse_glider")){
    return &conway_mouse_glider;
  }

  return NULL;
}

/* The behaviour of a console process can be summarised as an infinite 
 * loop over three main steps, namely
 *
 * 1. write a command prompt then read a command,
 * 2. split the command into space-separated tokens using strtok, then
 * 3. execute whatever steps the command dictates.
 *
 * As is, the console only recognises the following commands:
 *
 * a. execute <program name>
 *
 *    This command will use fork to create a new process; the parent
 *    (i.e., the console) will continue as normal, whereas the child
 *    uses exec to replace the process image and thereby execute a
 *    different (named) program.  For example,
 *    
 *    execute P3
 *
 *    would execute the user program named P3.
 *
 * b. terminate <process ID> 
 *
 *    This command uses kill to send a terminate or SIG_TERM signal
 *    to a specific process (identified via the PID provided); this
 *    acts to forcibly terminate the process, vs. say that process
 *    using exit to terminate itself.  For example,
 *  
 *    terminate 3
 *
 *    would terminate the process whose PID is 3.
 * 
 * c. nice <process ID> <priority>
 *    
 *    change the priority of process <proccess ID> to <priority>
 * 
 * d. ps
 * 
 *    draw the process tree
 * 
 * e. conway (start|line|glider|reset|mouse_glider)
 * 
 *    start: Begin running conways game of life
 * 
 *    line: Create a horizontal line along the center row without edge cells
 * 
 *    glider: Create a glider in the top left
 * 
 *    mouse_glider: Create a glider around where the mouse cursor is positioned
 */

void main_console_2() {
  char* p, x[ 1024 ];

  while( 1 ) {
    puts( "shell$ ", 7 ); gets( x, 1024 ); p = strtok( x, " " );

    if     ( 0 == strcmp( p, "execute"   ) ) {
      pid_t pid = fork();

      if( 0 == pid ) {
        exec( load_2( strtok( NULL, " " ) ) );
      }
    } 
    else if( 0 == strcmp( p, "terminate" ) ) {
      pid_t pid = atoi( strtok( NULL, " " ) );
      int   s   = atoi( strtok( NULL, " " ) );

      kill( pid, s );
    } 

    //Nice
    else if( 0 == strcmp( p, "nice" ) ) {
      pid_t pid = atoi( strtok( NULL, " " ) );
      int   x   = atoi( strtok( NULL, " " ) );

      nice( pid, x );
    } 

    //Process tree
    else if(0 == strcmp(p, "ps")){
      ps();
    }

    //Functions for conways game of life
    else if     ( 0 == strcmp( p, "conway"   ) ) {
      pid_t pid = fork();

      if( 0 == pid ) {
        exec( conway_load( strtok( NULL, " " ) ) );
      }
    } 
    else {
      puts( "unknown command\n", 16 );
    }
  }

  exit( EXIT_SUCCESS );
}
