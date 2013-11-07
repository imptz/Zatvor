#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc.h"
#include "timers.h"

TPointToHandlerADCInterrupt MfuncPoint[8];
unsigned char NumChannel=99;
void ADCExec(void);
void ADCTime(void);

void InitADC(void){
 char i;
 for (i=0;i<8;i++) {MfuncPoint[i]=0;}
 NumChannel=99;
 DDRC&=0xfe;
 PORTC&=0xfe;
 ADMUX=0;
 SFIOR&=0xef;
 ADCSR=0x8d;
 RegVirtualTimer(&ADCTime);
}

void ADCExec (void){
	do
		if (NumChannel==7){
			NumChannel=0;
		}else{
			NumChannel++;
		}
	while (MfuncPoint[NumChannel]==0);
	
	ADMUX&=0xf0;
	ADMUX|=NumChannel;
	ADCSR|=0x40;
}

void RegChannel(TPointToHandlerADCInterrupt fpoint,unsigned char channel){
	if (channel < 8){
		MfuncPoint[channel]=fpoint;
		if (NumChannel == 99){
			NumChannel=0;ADCExec();
		}
	}
}

ISR(ADC_vect){
(*MfuncPoint[NumChannel])();
}

void ADCTime(void) {ADCExec();}
