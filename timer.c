/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/
#include "defines.h"
// timer.c file
/***** timer confiuguration values *****/
#define CTL_ENABLE          ( 0x00000080 )
#define CTL_MODE            ( 0x00000040 )
#define CTL_INTR            ( 0x00000020 )
#define CTL_PRESCALE_1      ( 0x00000008 )
#define CTL_PRESCALE_2      ( 0x00000004 )
#define CTL_CTRLEN          ( 0x00000002 )
#define CTL_ONESHOT         ( 0x00000001 )

int color = YELLOW;

volatile TIMER timer[4];  // 4 timers; 2 timers per unit; at 0x00 and 0x20
//extern TIMER *tp[4];
int kprintf(char *fmt, ...);
int kpchar(char, int, int);
int unkpchar(char, int, int);
extern volatile TIMER timer[4];  // 4 timers; 2 timers per unit; at 0x00 and 0x20

void timer_init()
{
  int i=0;
  TIMER *tp;
  kprintf("timer_init()\n");

  //for (i=0; i<4; i++)
  {
    tp = &timer[i];
    if (i==0) tp->base = (u32 *)0x101E2000; 
    if (i==1) tp->base = (u32 *)0x101E2020; 
    if (i==2) tp->base = (u32 *)0x101E3000; 
    if (i==3) tp->base = (u32 *)0x101E3020;
    //printf("%d: %x %x %x\n", i, tp->base, tp->base+TLOAD, tp->base+TVALUE);
    *(tp->base+TLOAD) = 0x0;   // reset
    *(tp->base+TVALUE)= 0xFFFFFFFF;
    *(tp->base+TRIS)  = 0x0;
    *(tp->base+TMIS)  = 0x0;
    *(tp->base+TLOAD) = 0x100;
    *(tp->base+TCNTL) = 0x62; //011-0000=|NOTEn|Pe|IntE|-|scal=00|32-bit|0=wrap|
    *(tp->base+TBGLOAD) = 3817;  //0xF0000;

    tp->tick = tp->hh = tp->mm = tp->ss = 0;
    strcpy((char *)tp->clock, "00:00:00");
  }
}

void timer_handler(int n) {
    u32 ris, mis, value, load, bload, i;
    TIMER *t = &timer[n];
    // read all status info
    ris   = *(t->base+TRIS);
    mis   = *(t->base+TMIS);
    value = *(t->base+TVALUE);
    load  = *(t->base+TLOAD);
    bload = *(t->base+TBGLOAD);

    t->tick++; t->ss = t->tick;
    t->ss %= 60;
    if (t->ss==0){
      t->mm++;
      if ((t->mm % 60)==0){
        t->mm = 0;
	t->hh++;
      }
    }
    for (i=0; i<8; i++){
      unkpchar(t->clock[i], n, 70+i);
    }
    t->clock[7]='0'+(t->ss%10); t->clock[6]='0'+(t->ss/10);
    t->clock[4]='0'+(t->mm%10); t->clock[3]='0'+(t->mm/10);
    t->clock[1]='0'+(t->hh%10); t->clock[0]='0'+(t->hh/10);
 
    color = n+1;
    for (i=0; i<8; i++){
       kpchar(t->clock[i], n, 70+i);
    }
    TCPClockHandler();
    timer_clearInterrupt(n);
    return;
} 
T0TC(){
	return *(timer[0].base+TVALUE);
}
void timer_start(int n) // timer_start(0), 1, etc.
{
  TIMER *tp = &timer[n];

  kprintf("timer_start %d base=%x\n", n, tp->base);
  *(tp->base+TCNTL) |= 0x80;  // set enable bit 7
}

int timer_clearInterrupt(int n) // timer_start(0), 1, etc.
{
  TIMER *tp = &timer[n];
  *(tp->base+TINTCLR) = 0xFFFFFFFF;
}

void timer_stop(int n) // timer_start(0), 1, etc.
{
  TIMER *tp = &timer[n];
  *(tp->base+TCNTL) &= 0x7F;  // clear enable bit 7
}
