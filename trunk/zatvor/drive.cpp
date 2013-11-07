#include <avr/io.h>

#include "drive.h"
#include "timers.h"
#include "adc.h"
#include "main.h"
#include "lamp.h"

void DriveTimeFunc(void);
void ADCCurrent0(void);
void ADCCurrent1(void);
void ADCVoltage(void);

unsigned char drivePhase;
unsigned int driveStopCounter;
unsigned int driveDirect;

#define bridge_off PORTD&=0x7f;PORTB&=0xf8
#define bridge_down_on PORTD&=0x7f;PORTB&=0xfe;PORTB|=0x06
#define bridge_start_1_0 PORTD&=0x7f;PORTB&=0xfb;PORTB|=0x01
#define bridge_start_2_0 PORTB&=0xfc;PORTD|=0x80;
#define bridge_start_1_1 PORTD&=0x7f;PORTB&=0xfb;PORTB|=0x03
#define bridge_start_2_1 PORTB&=0xfc;PORTD|=0x80;PORTB|=0x04

unsigned int current[2];
unsigned int voltage;

unsigned int MaxCurrent0[2];
unsigned int countTimeMaxCurrent0[2];

unsigned int MaxCurrent1[2];
unsigned int countTimeMaxCurrent1[2];

unsigned char f_excessCurrent;
unsigned char f_excessCurrentCache;

unsigned char driveStartCounter;
#define const_driveStartCounter 3

void TestMaxCurrent(char,unsigned int);
void CalculateVoltage(unsigned int);


void DriveInit(void){
	DDRD|=0x80;DDRB|=0x07;PORTD&=0x7f;PORTB&=0xf8;
	bridge_off;
	driveStopCounter=0;
	drivePhase=0;
	driveDirect=0;
	RegVirtualTimer(&DriveTimeFunc);
	RegChannel(&ADCVoltage,ADCChannel_0);
	RegChannel(&ADCCurrent0,ADCChannel_6);
	RegChannel(&ADCCurrent1,ADCChannel_7);
	MaxCurrent0[0]=initMaxCurrent0;MaxCurrent0[1]=initMaxCurrent0;
	MaxCurrent1[0]=initMaxCurrent1;MaxCurrent1[1]=initMaxCurrent1;
	countTimeMaxCurrent0[0]=0;countTimeMaxCurrent0[1]=0;
	countTimeMaxCurrent1[0]=0;countTimeMaxCurrent1[1]=0;
	f_excessCurrent=0;
	voltage=0;
	current[0]=0;
	current[1]=0;
}

unsigned char DriveMove(unsigned char p){
	// 2 - close, 1 - open
	switch (p)
	{
		case 0:
			if ((driveDirect!=1)&&(driveDirect!=2)) {return 0;}
			else if (driveDirect==0) {return 1;}
			{
				drivePhase=1;
				driveDirect=const_driveStopCounter;
				return 1;
			}
			break;
	case 1:
	if ((driveDirect!=0)&&(driveDirect!=1)) {return 0;}
	else if (driveDirect==1) {return 1;}
	{
		drivePhase=4;
		driveDirect=1;
		bridge_start_1_0;
		driveStartCounter=const_driveStartCounter;
		return 1;
	}
	break;
	case 2:
	if ((driveDirect!=0)&&(driveDirect!=2)) {return 0;}
	else if (driveDirect==2) {return 1;}
	{
		drivePhase=5;
		driveDirect=2;
		bridge_start_2_0;
		driveStartCounter=const_driveStartCounter;
		return 1;
	}
	break;
	}
	return 0;
}

void DriveTimeFunc(void)
{
 switch (drivePhase)
 {
   case 0:
    break;
   case 1:
    bridge_off;
    drivePhase=2;
    break;
   case 2:
    bridge_down_on;
    driveStopCounter=0;
    drivePhase=3;
    break;
   case 3:
    if (driveStopCounter==const_driveStopCounter-10)
    {
      bridge_off;
      drivePhase=10;
    }
    else driveStopCounter++;
    break;
   case 4:
     if (driveStartCounter--==0) {drivePhase=6;}
     break;
   case 5:
     if (driveStartCounter--==0) {drivePhase=7;}
     break;
   case 6:
    drivePhase=0;
    bridge_start_1_1;
    break;
   case 7:
    drivePhase=0;
    bridge_start_2_1;
    break;
   default:
    if (driveStopCounter==const_driveStopCounter)
    {
      drivePhase=0;
      driveDirect=0;
    }
    else driveStopCounter++;
    break;
 }
}

void TestMaxCurrent(char n,unsigned int t)
{
 if (t>MaxCurrent0[n])
  if (countTimeMaxCurrent0[n]==const_countTimeMaxCurrent0)
   {
    countTimeMaxCurrent0[n]=0;
    f_excessCurrent=1;
    f_excessCurrentCache=1;
   }
  else countTimeMaxCurrent0[n]++;
 else countTimeMaxCurrent0[n]=0;

 if (t>MaxCurrent1[n])
  if (countTimeMaxCurrent1[n]==const_countTimeMaxCurrent1)
   {
    countTimeMaxCurrent1[n]=0;
    f_excessCurrent=1;
    f_excessCurrentCache=1;
   }
  else countTimeMaxCurrent1[n]++;
 else countTimeMaxCurrent1[n]=0;
}

void CalculateVoltage(unsigned int t)
{
 voltage=t;
}

void ADCCurrent0(void)
{
 int temp;
 temp=0;temp=ADCL;temp|=ADCH << 8;
 temp*=10;temp/=27;
 current[0]=temp;
 TestMaxCurrent(0,temp);
}

void ADCCurrent1(void)
{
 int temp;
 temp=0;temp=ADCL;temp|=ADCH << 8;
 temp*=10;temp/=27;
 current[1]=temp;
 TestMaxCurrent(1,temp);
}

void ADCVoltage(void)
{
 int temp;
 temp=0;temp=ADCL;temp|=ADCH << 8;temp/=2;
 CalculateVoltage(temp);
}

void Clearf_excessCurrent(void)
{
  f_excessCurrent=0;
}
