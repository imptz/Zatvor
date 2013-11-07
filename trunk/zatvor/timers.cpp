#include <avr/io.h>
#include <avr/interrupt.h>
#include "timers.h"

#define maxNumberTimers 10

TPointToHandlerVirtualTimers VirtualTimers[maxNumberTimers];
unsigned char NumTimers=0;

void InitVirtualTimers(void)
{
 char i;
 for (i=0;i<maxNumberTimers-1;i++) {VirtualTimers[i]=0;}
 NumTimers=0;
 TCCR2=0x0b;
 OCR2=115;
 TIMSK|=0x80;
}

ISR(TIMER2_COMP_vect){
 char i;
 for (i=0;i<NumTimers;i++) {(*VirtualTimers[i])();}
}

void RegVirtualTimer(TPointToHandlerVirtualTimers fpoint)
{
 if (NumTimers<maxNumberTimers) {VirtualTimers[NumTimers]=fpoint;NumTimers++;}
}
