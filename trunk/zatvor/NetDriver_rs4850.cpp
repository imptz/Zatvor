#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdarg.h>
#include "main.h"
#include "NetDriver_rs4850.h"
#include "timers.h"
#include "drive.h"
#include "but.h"
#include "sensors.h"
#include "lamp.h"

void rs485_StartSend(void);
void rs485_SendFunc(void);
void rs485_SendControlSum(void);

unsigned char rs485_ReceiveControlSum(void);
void rs485_ControlTimeRecByte(void);
void rs485_PeriodicNetDriverFunc(void);

unsigned char rs485_deviceAddress=0;
TPointer_HandlerRecPackage rs485_pointer_HandlerRecPackage;
TPointer_SetUpPortUartFunc rs485_pointer_SetUpPortUartFunc;
TPointer_RecToSendFunc rs485_pointer_RecToSendFunc;
TPointer_SendToRecFunc rs485_pointer_SendToRecFunc;
#define rs485_SetUpPortUart (*rs485_pointer_SetUpPortUartFunc)();
#define rs485_RecToSend (*rs485_pointer_RecToSendFunc)();UCSRB|=0x08;UCSRB&=0xef;
#define rs485_SendToRec (*rs485_pointer_SendToRecFunc)();UCSRB|=0x10;UCSRB&=0xfe;

#define FRAMING_ERROR (0x10)
#define DATA_OVERRUN  (0x08)

unsigned char rs485_receiveBuf[21]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char rs485_r_count=0;
unsigned char rs485_rec_control_count=0;  // счетчик контроля времени приема
unsigned char rs485_f_rec_control_count=0;  // флаг контроля времени приема 1 - вкл, 0 - выкл
#define rs485_const_rec_control_count (10) // константа счетчика контроля приема

unsigned char rs485_sendBuf[21]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char rs485_s_count=0;
unsigned char rs485_f_sending=0;


unsigned char rs485_bVal[4];

void rs485_InitNetDriver_RS485(char lDeviceAddress,char ubh,char ubl, TPointer_HandlerRecPackage lPointer_HandlerRecPackage,
	 				  	 TPointer_SetUpPortUartFunc lPointer_SetUpPortUartFunc,
						 TPointer_RecToSendFunc lPointer_RecToSendFunc,
						 TPointer_SendToRecFunc lPointer_SendToRecFunc)
{
 UCSRA=0x00;
 UCSRB=0xd0;
 UCSRC=0x86;
 UBRRH=ubh;
 UBRRL=ubl;
 rs485_deviceAddress=lDeviceAddress;
 rs485_pointer_HandlerRecPackage=lPointer_HandlerRecPackage;
 rs485_pointer_SetUpPortUartFunc=lPointer_SetUpPortUartFunc;
 rs485_pointer_RecToSendFunc=lPointer_RecToSendFunc;
 rs485_pointer_SendToRecFunc=lPointer_SendToRecFunc;
 rs485_SetUpPortUart
 rs485_SendToRec
 RegVirtualTimer(&rs485_PeriodicNetDriverFunc);
}

void rs485_FillPackage(char da,char sa,char com,char npar, ...)
{
 char i;
 va_list par;
 va_start(par,npar);
 rs485_sendBuf[0]=da;
 rs485_sendBuf[1]=sa;
 rs485_sendBuf[2]=com;
 rs485_sendBuf[3]=npar;
 i=4;
 for (i=4;i<(4+npar);i++)
  {
   int p = va_arg(par,int);
   rs485_sendBuf[i]=p;
  }
 rs485_sendBuf[20]=1;
 rs485_StartSend();
 va_end(par);
}

void rs485_FillPackage_1(char da,char sa,char com,char npar, ...)
{
 char i;
 if (rs485_sendBuf[20]==0)
  {
   va_list par;
   va_start(par,npar);
   rs485_sendBuf[0]=da;
   rs485_sendBuf[1]=sa;
   rs485_sendBuf[2]=com;
   rs485_sendBuf[3]=npar;
   i=4;
   for (i=4;i<(4+npar);i++)
    {
     int p = va_arg(par,int);
     rs485_sendBuf[i]=p;
    }
   rs485_sendBuf[20]=1;
   va_end(par);
  }
}

void rs485_SendControlSum(void)
{
 char i,j,k;
 j=rs485_sendBuf[3]+4;
 k=0;
 for (i=0;i<j;i++) {k+=rs485_sendBuf[i];}
 rs485_sendBuf[j]=k;
}

void rs485_StartSend(void)
{
 if ((rs485_f_sending==0)&&(rs485_sendBuf[20]!=0))
  {
   PORTC|=0x20;
   rs485_f_sending=1;
   rs485_SendControlSum();
   rs485_s_count=0;
   rs485_SendFunc();
  }
}

void rs485_SendFunc(void)
{
 if (rs485_f_sending==1)
  {
   if ((rs485_s_count!=(rs485_sendBuf[3]+5)) && (rs485_s_count<20))
    {rs485_RecToSend delay_mks_10(10);UDR=rs485_sendBuf[rs485_s_count];rs485_s_count++;}
   else {rs485_SendToRec rs485_sendBuf[20]=0;rs485_f_sending=0;PORTC&=0xdf;}
  }
}

ISR(USART0_TX_vect){
 rs485_SendFunc();
}

/////////////////////////////////////////////////////////////////////////////////////

unsigned char rs485_ReceiveControlSum(void)
{
 char i,j,k;
 j=rs485_receiveBuf[3]+4;if (j>19) {return (0);}
 k=0;
 for (i=0;i<j;i++) {k+=rs485_receiveBuf[i];}
 if (k==rs485_receiveBuf[j]) {return (1);} else {return (0);}
}

ISR(USART0_RX_vect){
 char status;
 status=UCSRA;
 rs485_rec_control_count=0;
 if ((status & ((FRAMING_ERROR)|(DATA_OVERRUN)))==0)
  {
   status=UDR;
   if (rs485_r_count==0)
   {
     if (status==rs485_deviceAddress)
     {
       rs485_receiveBuf[rs485_r_count]=status;
       rs485_r_count=1;
     }
   }
   else
    {
     rs485_receiveBuf[rs485_r_count]=status;
     rs485_r_count++;
     if (rs485_r_count>=5)
      if ((rs485_r_count==5+rs485_receiveBuf[3])||(rs485_r_count==21))
       {
        rs485_r_count=0;
        if (rs485_ReceiveControlSum()==1) {if (*rs485_pointer_HandlerRecPackage!=0) {(*rs485_pointer_HandlerRecPackage)();}}
       }
    }
  } else {status=UDR;}
}

void rs485_ControlTimeRecByte(void)
{
 if (rs485_rec_control_count==(rs485_const_rec_control_count-1))
  {
   rs485_rec_control_count=rs485_const_rec_control_count;
   rs485_r_count=0;
  }
 else if (rs485_rec_control_count!=rs485_const_rec_control_count) {rs485_rec_control_count++;}
}

void rs485_PeriodicNetDriverFunc(void)  // период вызова ~1 ms
{
 rs485_ControlTimeRecByte();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void rs485_RecPackageFunc(void);
void rs485_SetUpPortUartFunc(void);
void rs485_RecToSendFunc(void);
void rs485_SendToRecFunc(void);

char rs485_lsAddress;

void rs485_InitNetwork(void)
{
// unsigned char t1,t2;
 bVal[0]=1;bVal[2]=1;

 DDRB&=0xc7;PORTB|=0x38;
 DDRD&=0xe3;PORTD|=0x1c;
 asm("nop");asm("nop");asm("nop");asm("nop");
 rs485_lsAddress=(~PIND)&0x1c;
 rs485_lsAddress=rs485_lsAddress>>1;
 rs485_lsAddress=rs485_lsAddress+(((~PINB)&0x08)>>3);
 rs485_lsAddress=rs485_lsAddress+(((~PINB)&0x20)>>1);
 rs485_lsAddress=rs485_lsAddress+(((~PINB)&0x10)<<1);
// t1=0;
// t1|=((rs485_lsAddress&0x01)<<3);
// t1|=((rs485_lsAddress&0x02)<<1);
// t1|=((rs485_lsAddress&0x04)>>1);
// t1|=((rs485_lsAddress&0x08)>>3);
// rs485_lsAddress=t1;
 rs485_InitNetDriver_RS485(rs485_lsAddress,0,3,&rs485_RecPackageFunc,&rs485_SetUpPortUartFunc,&rs485_RecToSendFunc,&rs485_SendToRecFunc);
}

void rs485_RecPackageFunc(void)
{
 unsigned char temp;
 unsigned int temp1;
 switch (rs485_receiveBuf[2])
  {
/*---------- Network controller --------------------------------------------*/
  case 9:
   zatvorOpen();
   rs485_FillPackage(rs485_receiveBuf[1],rs485_receiveBuf[0],120,1,9);
   break;
  case 10:
   zatvorClose();  
   rs485_FillPackage(rs485_receiveBuf[1],rs485_receiveBuf[0],120,1,10);
   break;
  case 11:
   zatvorStop();   
   rs485_FillPackage(rs485_receiveBuf[1],rs485_receiveBuf[0],120,1,11);
   break;
  case 159:   
   SetOuts(rs485_receiveBuf[4]);
   rs485_FillPackage(rs485_receiveBuf[1],rs485_receiveBuf[0],120,1,159);
   break;
  case 158:
   temp=0;
   temp=0;
  
   switch (getSensorState())
   {
    case SENSOR_STATE_OPEN:
     temp = 0x10;
	 break;
    case SENSOR_STATE_CLOSE:
     temp = 0x04;
	 break;
    case SENSOR_STATE_WORK:
     temp = 0x00;
	 break;
    case SENSOR_STATE_FAULT:
	  temp = 0x16;
	 break;
   }
   
   temp|=f_excessCurrentCache;
   f_excessCurrentCache=0;
   if (driveDirect==1) {temp1=current[1];}
   else if (driveDirect==2) {temp1=current[0];}
   else {temp1=0;}
   temp |= outsState;
   rs485_FillPackage(rs485_receiveBuf[1],rs485_receiveBuf[0],120,3,158,temp,temp1);
   break;
  }
}

void rs485_SetUpPortUartFunc(void) {DDRC|=0x20;}
void rs485_RecToSendFunc(void) {PORTC|=0x20;}
void rs485_SendToRecFunc(void) {PORTC&=0xdf;}










