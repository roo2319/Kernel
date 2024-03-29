/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

//Access as fb[r][c]
char procs = 1; pcb_t pcb[ 100 ]; pcb_t* current = NULL;char nextpid = 2; uint16_t fb[ 600 ][ 800 ]; coord_t mouse;
coord_t cursor; unsigned int randomnumber = 35281;

//Convert character to be readable by font
char to_qwerty(char x){
  switch(x){
    case 0 : return 10; //A
    case 1 : return 23; //B
    case 2 : return 21; //C
    case 3 : return 12; //D
    case 4 : return  2; //E
    case 5 : return 13; //F
    case 6 : return 14; //G
    case 7 : return 15; //H
    case 8 : return 7 ; //I
    case 9 : return 16; //J
    case 10: return 17; //K
    case 11: return 18; //L
    case 12: return 25; //M
    case 13: return 24; //N
    case 14: return  8; //O
    case 15: return  9; //P
    case 16: return  0; //Q
    case 17: return  3; //R
    case 18: return 11; //S
    case 19: return  4; //T
    case 20: return  6; //U
    case 21: return 22; //V
    case 22: return  1; //W
    case 23: return 20; //X
    case 24: return  5; //Y
    case 25: return 19; //Z
    default: return -1;
  }
}

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

//Helper function to print to uart
void print(PL011_t* uart, char* s){
  char ind = 0;
  char c = s[0];
  while (c != '\0'){
    PL011_putc(uart,c,true);
    ind++;
    c=s[ind];
  }
}



/*It's not as bad as it sounds!
* Recursively kill the children of a parent process
*/
void kill_children(pid_t parent){
  pid_t child_pid;
  char out[2];
  for (int i = 0; i<procs;i++){
    if (pcb[i].parent == parent){

      //Store the pid of the child
      child_pid = pcb[i].pid;
      //Kill the childs children
      kill_children(child_pid);
      itoa_k(out, child_pid);

      //Kill the child itself (Similar implementation to kill)
      //Note that we are iterating again as we can't guarantee pcb state
      for (int j = 0; j<procs; j++){
        if (pcb[j].pid == child_pid && j != (procs - 1)){
          memcpy(&pcb[j],&pcb[procs-1],sizeof(pcb_t));
          memset(&pcb[procs-1],0,sizeof(pcb_t));
          procs--;
          print(UART1,"\nTerminated process ");
          print(UART1,out);
          PL011_putc(UART1,'\n',true);
        }

        else if (pcb[j].pid == child_pid){
          memset(&pcb[procs-1],0,sizeof(pcb_t));
          procs--;
          print(UART1, "\nTerminated process ");
          print(UART1, out);
          PL011_putc(UART1,'\n',true);
          }
      }
      //Reset iterator, PCB state is not guarunteed anymore
      i = 0;
    }
  }

}

//Recursively draw the children of a process
//Necessary for process tree
void draw_children(pid_t parent,char depth){
  pid_t child_pid;
  char out[2];
  for (int i = 0; i<procs;i++){
    if (pcb[i].parent == parent){
      child_pid = pcb[i].pid;
      for (int j = 0; j<depth;j++){
        print(UART1,"---");
      }
      PL011_putc(UART1,' ',true);
      itoa_k(out, child_pid);
      print(UART1,out);
      PL011_putc(UART1,'\n',true);
      //Draw the childs children
      draw_children(child_pid,depth+1);
    }
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

//A priority + age based scheduler.
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
  //Increments the age of idle processess
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

extern void     main_console_2();
extern uint32_t tos_console;




void hilevel_handler_rst(ctx_t* ctx) {
  // Configure the LCD display into 800x600 SVGA @ 36MHz resolution.

  SYSCONF->CLCD      = 0x2CAC;     // per per Table 4.3 of datasheet
  LCD->LCDTiming0    = 0x1313A4C4; // per per Table 4.3 of datasheet
  LCD->LCDTiming1    = 0x0505F657; // per per Table 4.3 of datasheet
  LCD->LCDTiming2    = 0x071F1800; // per per Table 4.3 of datasheet

  LCD->LCDUPBASE     = ( uint32_t )( &fb );

  LCD->LCDControl    = 0x00000020; // select TFT   display type
  LCD->LCDControl   |= 0x00000008; // select 16BPP display mode
  LCD->LCDControl   |= 0x00000800; // power-on LCD controller
  LCD->LCDControl   |= 0x00000001; // enable   LCD controller

  /* Configure the mechanism for interrupt handling by
   *
   * - configuring then enabling PS/2 controllers st. an interrupt is
   *   raised every time a byte is subsequently received,
   * - configuring GIC st. the selected interrupts are forwarded to the 
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */

  PS20->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
  PS20->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)
  PS21->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
  PS21->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)

  uint8_t ack;

        PL050_putc( PS20, 0xF4 );  // transmit PS/2 enable command
  ack = PL050_getc( PS20       );  // receive  PS/2 acknowledgement
        PL050_putc( PS21, 0xF4 );  // transmit PS/2 enable command
  ack = PL050_getc( PS21       );  // receive  PS/2 acknowledgement

  GICC0->PMR         = 0x000000F0; // unmask all          interrupts
  GICD0->ISENABLER1 |= 0x00300000; // enable PS2          interrupts
  GICC0->CTLR        = 0x00000001; // enable GIC interface
  GICD0->CTLR        = 0x00000001; // enable GIC distributor
  
  init_display(fb,&mouse,&cursor);

  /* Initialise two PCBs, representing user processes stemming from execution 
   * of two user programs.  Note in each case that
   *    
   * - the CPSR value of 0x50 means the processor is switched into USR mode, 
   *   with IRQ interrupts enabled, and
   * - the PC and SP values match the entry point and top of stack. 
   */


  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );     // initialise Console
  pcb[ 0 ].pid      = 1;
  pcb[ 0 ].parent   = -1;                      // Parent will never be called, so given dummy value
  pcb[ 0 ].status   = STATUS_CREATED;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console_2 );
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

  int_enable_irq();

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
  else if     ( id == GIC_SOURCE_PS20 ) {
    handle_keyboard(fb,&cursor, &mouse);
    
  }
  else if( id == GIC_SOURCE_PS21 ) {
    handle_mouse_move(fb,&mouse);
  
  }


  

  // Step 5: write the interrupt identifier to signal we're done

  GICC0->EOIR = id;

  return;
}

void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  int_unable_irq();
  switch( id ){

    case 0x00 : {// 0x00 => yield()
      schedule( ctx );

      break;
    }

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

      //If there is no space left in the procs table, return fail
      if (procs == 100){
        ctx->gpr[0] = -1;
      }
      
      //There is no need to initialise pcb to 0, as all fields will be set.
      //Initialise a copy of the parent process
      pcb[ procs ].pid      = nextpid;
      pcb[ procs ].parent   = current->pid;
      pcb[ procs ].status   = STATUS_CREATED;
      pcb[ procs ].priority = current->priority;
      pcb[ procs ].age      = current->age;
      memcpy(&pcb[procs].ctx,ctx,sizeof(ctx_t));
      
      //The TOS is allocated based on the pid
      //After 1000 pids, we will not be able to allocate more stack space.
      //This is a limitation of this kernel, but could be improved by "recycling" stack space.
      pcb[ procs ].tos      = (uint32_t) (&tos_console - (nextpid * 0x10000));
      pcb[ procs ].ctx.sp   = current->ctx.sp - (nextpid * 0x10000);

      //Copy the stack contents from parent to the child
      memcpy((void*) pcb[procs].ctx.sp,(void *) ctx->sp,current->tos - ctx->sp);
      pcb[ procs ].ctx.gpr[0] = 0;

      //Return value is childs pid
      ctx->gpr[0] = nextpid;
      
      //Increment nextpid and size of procs table
      nextpid++;
      procs++; 

      
      break;
    }

    case 0x04 : { //0x04 => Exit()
      pid_t pid = current->pid;
      char out[2];
      char code[3]; 
      itoa_k(code,ctx->gpr[0]);
      itoa_k(out,pid);
      for (int i = 0; i<procs; i++){

        //PCB in middle, swap
        if ((pcb[i].pid == pid) && (i != (procs - 1))){
          memcpy(&pcb[i],&pcb[procs-1],sizeof(pcb_t));
          procs--;
          break;
        }

        //Last case, Decrement procs count only
        else if (pcb[i].pid == pid){
          procs--;
          break;
        }
      }

      //Special exit code, signalling a NULL pointer passed to exec
      if (ctx->gpr[0] == 255){
        print(UART1,"Failed to start program");
        dispatch(ctx,NULL,&pcb[0]);
        //Create command prompt again, as it will be overwritten.
        print(UART1,"\nshell$ ");
        int_enable_irq();
        return;
      }

      //After everything we can print the exit code to console
      print(UART1, "\nProcess ");
      print(UART1, out);
      print(UART1, " Exited with exit code ");
      print(UART1, code);
      print(UART1,"\nshell$ ");
      dispatch(ctx, NULL, &pcb[0]);
      break;
    }
       

    //Replace the lr to the address, and move stack pointer to tos, as old context is no longer needed
    case 0x05 : { //0x05 => Exec(Const void* x)
      if (ctx->gpr[0] != (uint32_t) NULL){
        ctx->lr = (uint32_t)(ctx->gpr[0]);
        ctx->sp = current->tos; 
        ctx->cpsr = 0x50;
        }
      else{
        exit(255);
      }
      break;
    }


    //Recursively kills the process with process id pid.
    //Does not use signal, as I have no mechanism to signal soft kill.
    case 0x06 : { //0x06 => Kill(pid, id)
      char out[2];
      pid_t pid = (pid_t) ctx->gpr[0];
      itoa_k(out, pid);
      int x = ctx->gpr[1];

      //Kill children first
      kill_children(pid);

      //Functionality to terminate the console
      if (pid == 1){
        procs--;
        print(UART1, "\nTerminated Console\n");
        //Trap, Execution cannot continue
        while(true){}
      }

    //Otherwise, Iterate over procs, and swap last proc with proc that is being deleted
    for (int i = 0; i<procs; i++){
      if ((pcb[i].pid == pid) && (i != (procs - 1))){
        memcpy(&pcb[i],&pcb[procs-1],sizeof(pcb_t));
        procs--;
        print(UART1,"\nTerminated process ");
        print(UART1, out);
        PL011_putc(UART1,'\n',true);
        int_enable_irq();
        return;
      }

      //Unless the proc being deleted is the last proc, in which case just shorten PCB
      else if (pcb[i].pid == pid){
        procs--;
        print(UART1, "\nTerminated process ");
        print(UART1, out);
        PL011_putc(UART1,'\n',true);
        int_enable_irq(); 
        return;}
    }

    //If we fall through to here, nothing has been terminated
    print(UART1,"No process was terminated\n");
      break;
    }

    //Change the priority of pid to x
    case 0x07 : { //0x07 => Nice(pid, x)

      for (int i = 0; i<procs; i++){
        if (pcb[i].pid == ctx->gpr[0]){
          char out[2];
          itoa_k(out,ctx->gpr[0]);
          pcb[i].priority=ctx->gpr[1];
          print(UART1,"Successfully changed priority of process ");
          print(UART1,out);
          PL011_putc(UART1,'\n',true);
          int_enable_irq();
          return;
        }
      }
      print(UART1,"Failed to change priority of process\n");
      break; 
    }

    case 0x08 : { //0x08 => ps(), Draw process tree
      print(UART1,"1\n"); 
      draw_children(1,1);
      break;
    }


    //Allows a program to write text to the screen
    case 0x09 : { //0x09 => display_put(x)
      char* string = (char*) ctx->gpr[0];
      int n        = ctx->gpr[1];
      int colour   = ctx->gpr[2];
      int x;
      for( int i = 0; i < n; i++ ) {
        x = string[i];

        if (x == '\0'){
          break;
        }
        else if (x == '\n'){
          handle_newline(fb,&cursor);
        }
        else if (x == ','){
          draw_char(fb,&cursor,26,colour);
        }
        else if (x == '.'){
          draw_char(fb,&cursor,27,colour);
        }
        else if (x == '\''){
          draw_char(fb,&cursor,28,colour);
        } 
        else if (x == ' '){
          draw_char(fb,&cursor,29,colour);
        }
        
        //Deal with alphabetical cases
        else if (x - 'A' < 26){
          draw_char(fb,&cursor,to_qwerty(x-'A'),colour);
        }
        else if ((x - 'a') <26){
          draw_char(fb,&cursor,to_qwerty(x-'a'),colour);
        }
      }
      break;

    }

    case 0x0A : { //0x0A, draw_rectangle(fb,x,y,lenx,leny,colour)
      draw_rectangle(fb,ctx->gpr[0],ctx->gpr[1],ctx->gpr[2],ctx->gpr[3],ctx->gpr[4]);
      break;
    }

    case 0x0B: {//0x0B get_mouse_x()
      ctx->gpr[0] = mouse.x;
      break;
    }

    case 0x0C: {//0x0B get_mouse_y()
      ctx->gpr[0] = mouse.y;
      break;
    }

    //LCG implemented in kernel. 
    case 0x0D: {
      randomnumber = ((1103515245 * randomnumber + 12345) % (1<<31));
      ctx->gpr[0] = randomnumber;
      break;
    }



    default   : { // 0x?? => unknown/unsupported
      break;
    }
  
  int_enable_irq();
  return;
  }
}
