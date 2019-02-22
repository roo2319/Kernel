/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"
#define PCBSIZE 3
/*
PCBsize is defined here as a constant.
*/

pcb_t pcb[ PCBSIZE ]; pcb_t* current = NULL;

void dispatch( ctx_t* ctx, pcb_t* prev, pcb_t* next ) {
  char prev_pid = '?', next_pid = '?';

  if( NULL != prev ) {
    memcpy( &prev->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
    prev_pid = '0' + prev->pid;
  }
  if( NULL != next ) {
    memcpy( ctx, &next->ctx, sizeof( ctx_t ) ); // restore  execution context of P_{next}
    next_pid = '0' + next->pid;
  }

    /*
    PL011_putc( UART0, '[',      true );
    PL011_putc( UART0, prev_pid, true );
    PL011_putc( UART0, '-',      true );
    PL011_putc( UART0, '>',      true );
    PL011_putc( UART0, next_pid, true );
    PL011_putc( UART0, ']',      true );
    */
    current = next;                             // update   executing index   to P_{next}

  return;
}

void schedule( ctx_t* ctx ) {

  char max = 0;
  char maxi = 0;
  char previ;
  for (int i = 0; i<PCBSIZE; i++){
    if (current->pid == pcb[i].pid){
      previ = i;
    }
    if (pcb[i].priority + pcb[i].age > max){
      max = pcb[i].priority + pcb[i].age;
      maxi = i;
    }
    else{
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
  /*
  if     ( current->pid == pcb[ 0 ].pid ) {
    dispatch( ctx, &pcb[ 0 ], &pcb[ 1 ] );      // context switch P_1 -> P_2

    pcb[ 0 ].status = STATUS_READY;             // update   execution status  of P_1 
    pcb[ 1 ].status = STATUS_EXECUTING;         // update   execution status  of P_2
  }
  else if( current->pid == pcb[ 1 ].pid ) {
    dispatch( ctx, &pcb[ 1 ], &pcb[ 0 ] );      // context switch P_2 -> P_1

    pcb[ 1 ].status = STATUS_READY;             // update   execution status  of P_2
    pcb[ 0 ].status = STATUS_EXECUTING;         // update   execution status  of P_1
  }
*/
}

extern void     main_P3(); 
extern uint32_t tos_P3;
extern void     main_P4(); 
extern uint32_t tos_P4;
extern void     main_P6();
extern uint32_t tos_P6;

void hilevel_handler_rst(ctx_t* ctx) {

  /* Initialise two PCBs, representing user processes stemming from execution 
   * of two user programs.  Note in each case that
   *    
   * - the CPSR value of 0x50 means the processor is switched into USR mode, 
   *   with IRQ interrupts enabled, and
   * - the PC and SP values match the entry point and top of stack. 
   */

  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );     // initialise 0-th PCB = P_3
  pcb[ 0 ].pid      = 1;
  pcb[ 0 ].status   = STATUS_CREATED;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_P3 );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_P3  );
  pcb[ 0 ].priority = 0;
  pcb[ 0 ].age      = 0;



  memset( &pcb[ 1 ], 0, sizeof( pcb_t ) );     // initialise 2-nd PCB = P_4
  pcb[ 1 ].pid      = 2;
  pcb[ 1 ].status   = STATUS_CREATED;
  pcb[ 1 ].ctx.cpsr = 0x50;
  pcb[ 1 ].ctx.pc   = ( uint32_t )( &main_P4 );
  pcb[ 1 ].ctx.sp   = ( uint32_t )( &tos_P4  );
  pcb[ 1 ].priority = 0;
  pcb[ 1 ].age      = 0;

  memset( &pcb[ 2 ], 0, sizeof( pcb_t ) );     // initialise 3-rd PCB = P_5
  pcb[ 2 ].pid      = 3;
  pcb[ 2 ].status   = STATUS_CREATED;
  pcb[ 2 ].ctx.cpsr = 0x50;
  pcb[ 2 ].ctx.pc   = ( uint32_t )( &main_P6 );
  pcb[ 2 ].ctx.sp   = ( uint32_t )( &tos_P6 );
  pcb[ 2 ].priority = 0;
  pcb[ 2 ].age      = 0;



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

    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
  }
