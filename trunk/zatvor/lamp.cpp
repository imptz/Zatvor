#include <avr/io.h>
#include <avr/interrupt.h>

#include "lamp.h"
#include "timers.h"
#include "main.h"

// lamp0 - close, lamp1 - open

// period unit - 1 ms
#define periodLampFlash 250
#define Lamp1_on PORTD|=0x20
#define Lamp1_off PORTD&=0xdf
#define Lamp0_on PORTD|=0x40
#define Lamp0_off PORTD&=0xbf


void LampTimeFunc(void);

unsigned int lPeriod;
unsigned int lState[2]; // 0 - no light, 1 - light, 2 - flash

void InitLAMP(void)
{
 DDRD|=0x60;
 PORTD&=0x9f;
 RegVirtualTimer(&LampTimeFunc);
 lState[0]=0;lState[1]=0;
 lPeriod=0;
}

void SetLamp(unsigned char lamp,unsigned char state)
{
#ifdef BUTTONS  
  switch (state)
  {
    case 0: lState[lamp]=0;break;
    case 1: lState[lamp]=periodLampFlash;break;
    case 2: lState[lamp]=periodLampFlash/2;break;
  }
#endif  
}

void LampTimeFunc(void)
{
 if (lPeriod<lState[0]) {Lamp0_on;}
 else if (lPeriod>lState[0]) {Lamp0_off;}
 if (lPeriod<lState[1]) {Lamp1_on;}
 else if (lPeriod>lState[1]) {Lamp1_off;}
 if (lPeriod++==periodLampFlash)  {lPeriod=0;}
}

