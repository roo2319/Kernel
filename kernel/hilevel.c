/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"


char procs = 1; pcb_t pcb[ 20 ]; pcb_t* current = NULL;char nextpid = 2;  

void itoa_k( char* r, int x ) {
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

void print(char* s){
  char ind = 0;
  char c = s[0];
  while (c != '\0'){
    PL011_putc(UART0,c,true);
    ind++;
    c=s[ind];
  }
}

void dispatch( ctx_t* ctx, pcb_t* prev, pcb_t* next ) {

  if( NULL != prev ) {
    memcpy( &prev->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
  }
  if( NULL != next ) {
    memcpy( ctx, &next->ctx, sizeof( ctx_t ) ); // restore  execution context of P_{next}
  }
    current = next;                             // update   executing index   to P_{next}
    PL011_putc(UART0,'\n',true);

  return;
}

void schedule( ctx_t* ctx ) {

  char max = 0;
  char maxi = 0;
  char previ;
  for (int i = 0; i<procs; i++){
    if (current->pid == pcb[i].pid){
      previ = i;
    }
    if (pcb[i].priority + pcb[i].age > max){
      max = pcb[i].priority + pcb[i].age;
      maxi = i;
    }
  }
  for (int i = 0; i<procs; i++){
    if (i != maxi){
      pcb[i].age++;
    }
  }
  //If they are the same, The current progress still has max priority
  if (maxi == previ){return;}

  pcb[maxi].age = 0;
  dispatch(ctx, &pcb[previ], &pcb[maxi]); //Context switch from previous state to the next 
  pcb[previ].status = STATUS_READY;
  pcb[maxi].status = STATUS_EXECUTING;
  return;
}

extern void     main_console();
extern uint32_t tos_console;

//Copy from top -1000 to top for 1000 bytes 



void hilevel_handler_rst(ctx_t* ctx) {

  /* Initialise two PCBs, representing user processes stemming from execution 
   * of two user programs.  Note in each case that
   *    
   * - the CPSR value of 0x50 means the processor is switched into USR mode, 
   *   with IRQ interrupts enabled, and
   * - the PC and SP values match the entry point and top of stack. 
   */


  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );     // initialise Console
  pcb[ 0 ].pid      = 1;
  pcb[ 0 ].status   = STATUS_CREATED;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_console );
  pcb[ 0 ].tos      = ( uint32_t )( &tos_console );
  pcb[ 0 ].priority = 0;
  pcb[ 0 ].age      = 0;



  /* Configure the mechanism for interrupt handling by
   *
   * - configuring timer st. it raises a (periodic) interrupt for each 
   *   timer tick,
   * - configuring GIC st. the selected interrupts are forwarded to the 
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */

  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  dispatch(ctx,NULL,&pcb[ 0 ]);

  return;
}

void hilevel_handler_irq(ctx_t* ctx) {
   // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    schedule(ctx);  TIMER0->Timer1IntClr = 0x01;  
  }


  

  // Step 5: write the interrupt identifier to signal we're done

  GICC0->EOIR = id;

  return;
}

void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  switch( id ){
    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );  
      char*  x = ( char* )( ctx->gpr[ 1 ] );  
      int    n = ( int   )( ctx->gpr[ 2 ] ); 

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }
      
      ctx->gpr[ 0 ] = n;

      break;
    }

    case 0x03 : { //0x03 => Fork()
      //realloc(pcb,sizeof(pcb_t) * procs);
      memset( &pcb[ procs ], 0, sizeof( pcb_t ) );   
      pcb[ procs ].pid      = nextpid;
      pcb[ procs ].status   = STATUS_CREATED;
      pcb[ procs ].priority = current->priority;
      pcb[ procs ].age      = current->age;
      memcpy(&pcb[procs].ctx,ctx,sizeof(ctx_t));
      pcb[ procs ].tos      = (uint32_t) (&tos_console - (nextpid * 0x1000));
      pcb[ procs ].ctx.sp   = current->ctx.sp - (nextpid * 0x1000);
      memcpy(pcb[procs].ctx.sp,(current->ctx.sp),current->tos - current->ctx.sp);
      nextpid++;
      procs++; 
      pcb[ procs ].ctx.gpr[0] = 0;
      
      ctx->gpr[0] = current->pid;
      break;
    }

    case 0x04 : { //0x04 => Exit()
      char out[2];
      pid_t pid = current->pid;
      itoa_k(out, pid);
      for (int i = 0; i<procs; i++){
      if ((pcb[i].pid == pid) && (i != (procs - 1))){
        memcpy(&pcb[i],&pcb[procs-1],sizeof(pcb_t));
        memset(&pcb[procs-1],0,sizeof(pcb_t));
        procs--;
        print("\nIntermediate terminated process ");
        print(out);
        dispatch(ctx, NULL, &pcb[0]);
        return;
      }
      else{if (pcb[i].pid == pid){
        memset(&pcb[procs-1],0,sizeof(pcb_t));
        procs--;
        print("\nFinally terminated ");
        print(out); 
        dispatch(ctx, NULL, &pcb[0]);
        return;}}
      }
      print("\nNothing was terminated\n");
      break;
    }
       

    case 0x05 : { //0x05 => Exec(Const void* x)
      ctx->lr = (uint32_t)(ctx->gpr[0]);
      break;
    }

    case 0x06 : { //0x06 => Kill(pid, id)
    char out[2];
    pid_t pid = (pid_t) ctx->gpr[0];
    itoa_k(out, pid);
    int x = ctx->gpr[1];
    for (int i = 0; i<procs; i++){
      if ((pcb[i].pid == pid) && (i != (procs - 1))){
        memcpy(&pcb[i],&pcb[procs-1],sizeof(pcb_t));
        memset(&pcb[procs-1],0,sizeof(pcb_t));
        procs--;
        print("\nIntermediate terminated process ");
        print(out);
        PL011_putc(UART0,'\n',true); 
        return;
      }
      else{if (pcb[i].pid == pid){
        memset(&pcb[procs-1],0,sizeof(pcb_t));
        procs--;
        print("\nFinally terminated ");
        print(out);
        PL011_putc(UART0,'\n',true); 
        return;}}
    }
    print("No process was terminated");
      break;
    }

    case 0x07 : { //0x07 => Nice()
      break; 
    }

    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
  }
