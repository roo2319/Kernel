/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "libc.h"

int  atoi( char* x        ) {
  char* p = x; bool s = false; int r = 0;

  if     ( *p == '-' ) {
    s =  true; p++;
  }
  else if( *p == '+' ) {
    s = false; p++;
  }

  for( int i = 0; *p != '\x00'; i++, p++ ) {
    r = s ? ( r * 10 ) - ( *p - '0' ) :
            ( r * 10 ) + ( *p - '0' ) ;
  }

  return r;
}

void itoa( char* r, int x ) {
  char* p = r; int t, n;

  if( x < 0 ) {
     p++; t = -x; n = t;
  }
  else {
          t = +x; n = t;
  }

  do {
     p++;                    n /= 10;
  } while( n );

    *p-- = '\x00';

  do {
    *p-- = '0' + ( t % 10 ); t /= 10;
  } while( t );

  if( x < 0 ) {
    *p-- = '-';
  }

  return;
}

void yield() {
  asm volatile( "svc %0     \n" // make system call SYS_YIELD
              :
              : "I" (SYS_YIELD)
              : );

  return;
}

int write( int fd, const void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_WRITE
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "I" (SYS_WRITE), "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int  read( int fd,       void* x, size_t n ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_READ
                "mov %0, r0 \n" // assign r  = r0
              : "=r" (r) 
              : "I" (SYS_READ),  "r" (fd), "r" (x), "r" (n) 
              : "r0", "r1", "r2" );

  return r;
}

int  fork() {
  int r;

  asm volatile( "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0 
              : "=r" (r) 
              : "I" (SYS_FORK)
              : "r0" );

  return r;
}

void exit( int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  x
                "svc %0     \n" // make system call SYS_EXIT
              :
              : "I" (SYS_EXIT), "r" (x)
              : "r0" );

  return;
}

void exec( const void* x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 = x
                "svc %0     \n" // make system call SYS_EXEC
              :
              : "I" (SYS_EXEC), "r" (x)
              : "r0" );

  return;
}

int  kill( int pid, int x ) {
  int r;

  asm volatile( "mov r0, %2 \n" // assign r0 =  pid
                "mov r1, %3 \n" // assign r1 =    x
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r) 
              : "I" (SYS_KILL), "r" (pid), "r" (x)
              : "r0", "r1" );

  return r;
}

void nice( int pid, int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  pid
                "mov r1, %2 \n" // assign r1 =    x
                "svc %0     \n" // make system call SYS_NICE
              : 
              : "I" (SYS_NICE), "r" (pid), "r" (x)
              : "r0", "r1" );

  return;
}

void ps() {
  asm volatile( "svc %0     \n" // make system call SYS_PS
              :
              : "I" (SYS_PS)
              : );

  return;
}

void display_put( char* x, int n, int colour) {
  asm volatile( "mov r0, %1 \n" // assign r0 = x
                "mov r1,  %2 \n" // assign r1 = n
                "mov r2, %3  \n" // assign r2 = colour
                "svc %0     \n" // make system call SYS_DISPLAY_PUT
              :
              : "I" (SYS_DISPLAY_PUT), "r" (x), "r" (n), "r" (colour)
              : "r0", "r1", "r2");

  return;
}

void draw_rect( int x, int y, int xlen, int ylen, int colour) {
  asm volatile( "mov r0, %1 \n" // assign r0 = x
                "mov r1,  %2 \n" // assign r1 = y
                "mov r2, %3  \n" // assign r2 = xlen
                "mov r3, %4  \n" // assign r3 = ylen
                "mov r4, %5  \n" // assign r4 = colour
                "svc %0     \n" // make system call SYS_DISPLAY_PUT
              :
              : "I" (SYS_DRAW_RECT), "r" (x), "r" (y), "r" (xlen), "r" (ylen), "r" (colour)
              : "r0", "r1", "r2","r3","r4");

  return;
}

int  get_mouse_x() {
  int r;

  asm volatile( "svc %1     \n" // make system call SYS_GET_MOUSE_X
                "mov %0, r0 \n" // assign r  = r0 
              : "=r" (r) 
              : "I" (SYS_GET_MOUSE_X)
              : "r0" );

  return r;
}

int  get_mouse_y() {
  int r;

  asm volatile( "svc %1     \n" // make system call SYS_GET_MOUSE_Y
                "mov %0, r0 \n" // assign r  = r0 
              : "=r" (r) 
              : "I" (SYS_GET_MOUSE_Y)
              : "r0" );

  return r;
}

unsigned int  get_random() {
  int r;

  asm volatile( "svc %1     \n" // make system call SYS_GET_RANDOM
                "mov %0, r0 \n" // assign r  = r0 
              : "=r" (r) 
              : "I" (SYS_GET_RANDOM)
              : "r0" );

  return r;
}



void sem_post(void* sem){
 asm volatile("sem_post: ldrex  r1, [ r0 ]\n"       
                  "          add    r1, r1, #1\n"       
                  "          strex r2, r1, [ r0 ]\n"    
                  "          cmp    r2, #0\n"            
                  "          bne    sem_post\n"          
                  "          dmb\n");
  return;
}

void sem_wait(void* sem){
  asm volatile("sem_wait: ldrex  r1, [ r0 ]\n"
                   "          cmp    r1, #0 \n"
                   "          beq    sem_wait\n"
                   "          sub    r1, r1, #1\n"
                   "          strex r2, r1, [ r0 ]\n"
                   "          cmp    r2, #0\n"
                   "          bne    sem_wait\n"
                   "          dmb   \n");         
  return;
}

void sleep(unsigned int seconds){
   for( int i = 0; i < (seconds * 0x10000000); i++ ) {
        asm volatile( "nop" );
      }
}
