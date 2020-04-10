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
#include "vid.c"
#include "uart.c"
//#include "tcpip.h"
#include "lan91c111.h"    // Keil: *.c -> *.h    // ethernet packet driver
#include "tcpip.h"        // Keil: *.c -> *.h    // easyWEB TCP/IP stack
#include "easyweb.h"
 #include "webpage.h"
extern char _binary_panda_bmp_start;
extern char _binary_porkcar_bmp_start;

memcpy(char *p,char *p1,int len){
int i=0;
for (i=0;i<len;i++)
   *p++=*p1++;
} 
memset(char *p,char val,int len){
int i=0;
for (i=0;i<len;i++)
   *p++=val;
}
memcmp(char *p,char *p1,int len){
int i=0;
for (i=0;i<len;i++)
   if (*p++ != *p1++) return 1;
return 0;
} 
int color;
 // MAC address is stored to external EEPROM at read to registers   
    // after hard reset of LAN-controller   
   // unsigned char MyMAC[6];   // "M1-M2-M3-M4-M5-M6"  

// easyWEB internal function  
    // help function to swap the byte order of a WORD  
      
   
/*** this is for converted images with |h|w|pixels| format ****
int show(char *p, int startRow)
{ 
   int h, w, row, col, pixel; 
   unsigned char r, g, b;

   h = (*(p+3)<<24) + (*(p+2)<<16) + (*(p+1)<<8) + *p;
   p += 4;
   w = (*(p+3)<<24) + (*(p+2)<<16) + (*(p+1)<<8) + *p;
   p += 4;          // skip over 8 byes

   uprintf("h=%d w=%d\n", h, w);
   //  if (h > 480) h = 480;
   //if (w > 640) w = 640;

   row = startRow; col = 0;
   while(1){
     r = *p; g = *(p+1); b = *(p+2);
     pixel = (b<<16) + (g<<8) + r;
     //     fb[row*640 + col] = pixel;
     fb[row*WIDTH + col] = pixel;
     p += 3;         // advance p by 3 bytes
     col++;
     if (col >= w){  // to line width of jpg image
        col = 0;
        row++;
     }
     if (row >= h+startRow)
        break;
   }
}
*******************************************/
void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;

    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}
void timer_handler();

// IRQ interrupts handler entry point
//void __attribute__((interrupt)) IRQ_handler()
// timer0 base=0x101E2000; timer1 base=0x101E2020
// timer3 base=0x101E3000; timer1 base=0x101E3020
// currentValueReg=0x04
TIMER *tp[4];
void IRQ_handler()
{
    int vicstatus, sicstatus;
    int ustatus, kstatus;

    // read VIC SIV status registers to find out which interrupt
    vicstatus = VIC_STATUS;
    sicstatus = SIC_STATUS;
    // kprintf("vicstatus=%x sicstatus=%x\n", vicstatus, sicstatus);
    // VIC status BITs: timer0,1=4, uart0=13, uart1=14, SIC=31: KBD at 3

    if (vicstatus & 0x0010){   // timer0,1=bit4
      if (*(timer[0].base+TVALUE)==0) // timer 0
         timer_handler(0);
    //  if (*(timer[1].base+TVALUE)==0)
      //   timer_handler(1);
    }
   /* if (vicstatus & 0x0020){   // timer2,3=bit5
       if(*(tp[2]->base+TVALUE)==0)
         timer_handler(2);
       if (*(tp[3]->base+TVALUE)==0)
         timer_handler(3);
    }*/
}

int main()
{
   char c, *p;
   int mode;
   uart_init();
   up = upp[0];

   mode = 0;
   fbuf_init(mode);

   //p = &_binary_panda_bmp_start;
   //show_bmp(p, 0, 80);
   /* enable timer0,1, uart0,1 SIC interrupts */
      VIC_INTENABLE |= (1<<4);  // timer0,1 at bit4
Init91111();
HTTPStatus = 0;                                // clear HTTP-server's flag register

    TCPLocalPort = TCP_PORT_HTTP;                  // set port we want to listen to
   while(1){

	           //if (!(SocketStatus & SOCK_ACTIVE))
	        	  // TCPActiveOpen();   // listen for incoming TCP-connection
	   if (!(SocketStatus & SOCK_ACTIVE)) TCPPassiveOpen();
	        	//   delay1ms();
	           DoNetworkStuff();
	           // handle network and easyWEB-stack
	          // delay1ms();
	          // delay1ms(); // events
	          HTTPServer();


     //show_bmp(p, 120, 80); 
//  uprintf("enter a key from this UART : ");
  //   ugetc(up);

     //p = &_binary_porkcar_bmp_start;
   //  show_bmp(p, 0, 0);  
 //uprintf("enter a key from this UART : ");
   //  ugetc(up);

   }
   while(1);   // loop here  
}



// This function implements a very simple dynamic HTTP-server.
// It waits until connected, then sends a HTTP-header and the
// HTML-code stored in memory. Before sending, it replaces
// some special strings with dynamic values.
// NOTE: For strings crossing page boundaries, replacing will
// not work. In this case, simply add some extra lines
// (e.g. CR and LFs) to the HTML-code.

void HTTPServer(void)
{
  if (SocketStatus & SOCK_CONNECTED)             // check if somebody has connected to our TCP
  {
    if (SocketStatus & SOCK_DATA_AVAILABLE) { // check if remote TCP sent data


      TCPReleaseRxBuffer();                      // and throw it away
    }
    if (SocketStatus & SOCK_TX_BUF_RELEASED)     // check if buffer is free for TX
    {
      if (!(HTTPStatus & HTTP_SEND_PAGE))        // init byte-counter and pointer to webside
      {                                          // if called the 1st time
        HTTPBytesToSend = sizeof(WebSide) - 1;   // get HTML length, ignore trailing zero
        PWebSide = (unsigned char *)WebSide;     // pointer to HTML-code
      }

      if (HTTPBytesToSend > MAX_TCP_TX_DATA_SIZE)     // transmit a segment of MAX_SIZE
      {
        if (!(HTTPStatus & HTTP_SEND_PAGE))           // 1st time, include HTTP-header
        {
          memcpy(TCP_TX_BUF, GetResponse, sizeof(GetResponse) - 1);
          memcpy(TCP_TX_BUF + sizeof(GetResponse) - 1, PWebSide, MAX_TCP_TX_DATA_SIZE - sizeof(GetResponse) + 1);
          HTTPBytesToSend -= MAX_TCP_TX_DATA_SIZE - sizeof(GetResponse) + 1;
          PWebSide += MAX_TCP_TX_DATA_SIZE - sizeof(GetResponse) + 1;
        }
        else
        {
          memcpy(TCP_TX_BUF, PWebSide, MAX_TCP_TX_DATA_SIZE);
          HTTPBytesToSend -= MAX_TCP_TX_DATA_SIZE;
          PWebSide += MAX_TCP_TX_DATA_SIZE;
        }

        TCPTxDataCount = MAX_TCP_TX_DATA_SIZE;   // bytes to xfer
//        InsertDynamicValues();                   // exchange some strings...
        TCPTransmitTxBuffer();                   // xfer buffer
      }
      else if (HTTPBytesToSend)                  // transmit leftover bytes
      {
        memcpy(TCP_TX_BUF, PWebSide, HTTPBytesToSend);
        TCPTxDataCount = HTTPBytesToSend;        // bytes to xfer
        //InsertDynamicValues();                   // exchange some strings...
        TCPTransmitTxBuffer();                   // send last segment
        TCPClose();                              // and close connection
        HTTPBytesToSend = 0;                     // all data sent
      }

      HTTPStatus |= HTTP_SEND_PAGE;              // ok, 1st loop executed
    }
  }
  else
    HTTPStatus &= ~HTTP_SEND_PAGE;               // reset help-flag if not connected
}

