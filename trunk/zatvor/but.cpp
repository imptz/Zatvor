#include <avr/io.h>
#include <avr/interrupt.h>

#include "but.h"
#include "timers.h"
#include "lamp.h"

#define cBCounterMax  100
#define cBLevelMax    90
#define cBLevelMin    10

unsigned char f_bTrueVal;
unsigned int  bLCounter;
unsigned int  bLevel0[3];
unsigned int  bLevel1[3];
unsigned char bVal[3]; // 0/1 - on/off, 2 - invalid  index=0 - <close>, =1 - <stop>, =2 - <open>
unsigned char bTemp;


void ButtonsTimeFunc(void);

void InitButtons(void){
  f_bTrueVal=0;
  bLCounter=0;
  bLevel0[0]=0;bLevel0[1]=0;bLevel0[2]=0;
  bLevel1[0]=0;bLevel1[1]=0;bLevel1[2]=0;
  bVal[0]=1;bVal[1]=1;bVal[2]=1;
  RegVirtualTimer(&ButtonsTimeFunc);
  DDRC&=0xdf;PORTC&=0xdf;
  DDRD&=0xfc;PORTD&=0xfc;
}

void ButtonsTimeFunc(void){
//  bTemp = (PIND&0x01);
	bTemp = (PINC&0x20)>>5;
	switch (bTemp){
		case 0: bLevel0[0]++;break;
		case 1: bLevel1[0]++;break;
	}
  
	bTemp = (PIND&0x02)>>1;
	switch (bTemp){
		case 1: bLevel0[1]++;break;
		case 0: bLevel1[1]++;break;
	}
	bTemp = (PIND&0x01);
	//  bTemp = (PINC&0x20)>>5;
	switch (bTemp){
		case 0: bLevel0[2]++;break;
		case 1: bLevel1[2]++;break;
	}
  
	if (bLCounter!=cBCounterMax){
		bLCounter++;
	}
	else{
		f_bTrueVal=1;
		if (bLevel0[0]>=cBLevelMax){
			bVal[0]=0;
		}
		else
			if (bLevel1[0]>=cBLevelMax){
				bVal[0]=1;
			}else{
				bVal[0]=2;
			}
			
		if (bLevel0[1]>=cBLevelMax){
			bVal[1]=0;
		}else
			if (bLevel1[1]>=cBLevelMax){
				bVal[1]=1;
			}else{
				bVal[1]=2;
			}
			
		if (bLevel0[2]>=cBLevelMax){
			bVal[2]=0;
		}else
			if (bLevel1[2]>=cBLevelMax){
				bVal[2]=1;
			}else{
				bVal[2]=2;
			}
			
		bLCounter=0;
		bLevel0[0]=0;bLevel1[0]=0;
		bLevel0[1]=0;bLevel1[1]=0;
		bLevel0[2]=0;bLevel1[2]=0;
	}
}
