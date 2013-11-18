#include <avr/io.h>

#include "timers.h"
#include "lamp.h"
#include "sensors.h"
#include "but.h"
#include "drive.h"
#include "adc.h"
#include "network.h"
#include "main.h"

int moveCh;

unsigned int curOverCounter;
#define const_curOverCounter 2000

void Process(void);

unsigned char mainphase;

void initDelay(void);
char outsState;

char b0, b1,b2;


#ifndef BUTTONS
void SetOuts(unsigned char value){
 DDRD|=0x60;
 if ((value&0x01) == 0x01){
   PORTD|=0x40;outsState|=0x40;
   PORTD|=0x20;//outsState|=0x80;
 }else{
   PORTD&=0xbf;outsState&=0xbf;
   PORTD&=0xdf;//outsState&=0x7f;
 }
 
/* if ((value&0x02) == 0x02){
   PORTD|=0x20;outsState|=0x80;
 }else{
   PORTD&=0xdf;outsState&=0x7f;
 }*/
}
#endif

int main(void){
 long int i;
 asm("cli");
 //set WDT
 WDTCR|=(1<<4)|(1<<3);
 WDTCR=0x0d;
 asm("wdr");

 moveCh = 0;
 outsState = 0;
 
 InitVirtualTimers();
 initDelay();
#ifdef BUTTONS
 InitLAMP();
 InitButtons();
#endif 
 InitADC();
 InitSensors();
 DriveInit();
#ifndef BUTTONS
 Network::init();
 DDRD|=0x60;
#else
 SetLamp(0,1);
 SetLamp(1,1);
#endif
 asm("sei");
 // loop
 DriveMove(0);
 mainphase=0;
 
 b0 = 1;
 b1 = 1;
 b2 = 1;
 
 bVal[1] = 1;
 
 for (i=0;i<500000;i++) {asm("wdr");}
 while (true){
   asm("wdr");
#ifdef BUTTONS
   if (f_bTrueVal==1)
#endif     
   {
    RegVirtualTimer(&Process);
    break;
   }
 }
 
 while (true) {asm("wdr");}
}

void Process(void){
  enum SENSOR_STATE state = getSensorState();
  if ((state == SENSOR_STATE_FAULT) || (state == SENSOR_STATE_UNDEFINED))
	zatvorStop();
  
  if ((driveDirect == 1) && (state == SENSOR_STATE_OPEN))
	zatvorStop();
  if ((driveDirect == 2) && (state == SENSOR_STATE_CLOSE))
	zatvorStop();
  if (moveCh != 0){
	if (driveDirect == 0){
		DriveMove(moveCh);
		moveCh = 0;
	}
  }
}

void zatvorOpen(){
 if (DriveMove(1) == 0){
	 moveCh = 1;
	 DriveMove(0);
 }else
 	 moveCh = 0;
}

void zatvorClose(){
 if (DriveMove(2) == 0){
	 moveCh = 2;
	 DriveMove(0);
 }else
	 moveCh = 0;
}

void zatvorStop(){
 DriveMove(0);
 moveCh = 0;
}

void delay_mks_1000(int time){
 int i;int d;
 for (d=0;d<time+1;d++) {for (i=0;i<500;i++) {asm ("nop");}}
}

void delay_mks_100(int time){
 int i;int d;
 for (d=0;d<time+1;d++) {for (i=0;i<45;i++) {asm ("nop");}}
}

void delay_mks_10(int time){
 int i;int d;
 for (d=0;d<time+1;d++) {for (i=0;i<4;i++) {asm ("nop");}}
}

void delayFunc(void);
volatile char f_delay;
int mdelay;

void initDelay(void){
 mdelay=0;f_delay=0;
 RegVirtualTimer(&delayFunc);
}

void delayFunc(void){
 if (mdelay==0) {f_delay=0;} else mdelay--;
}

void delay(int n){
 f_delay=1;mdelay=n;while (f_delay){asm("wdr");};
}














