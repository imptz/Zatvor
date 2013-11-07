#include <avr/io.h>
#include <avr/interrupt.h>

#include "sensors.h"
#include "timers.h"
#include "drive.h"
#include "main.h"

enum SENSOR_STATE sensorState;
enum SENSOR_READY sensorReady;

unsigned int  pinCounter;
#define PIN_COUNTER_MAX 100

int pin0Value;
int pin1Value;

unsigned char f_invalideVoltage;
#define MINIMAL_VOLTAGE 150
unsigned int invalideVSensTimer;
#define const_InvalideVSensTimer 300

enum SENSOR_STATE getSensorState()
{
 return sensorState; 
}

enum SENSOR_READY getSensorReady()
{
 return sensorReady; 
}

unsigned char getPin0()
{
 return (PINC & 0x02 ) >> 1; 
}

unsigned char getPin1()
{
 return (PINC & 0x08 ) >> 3; 
}

void pin0ValueDec()
{
 if (pin0Value > -126)
   pin0Value--;
}

void pin0ValueInc()
{
 if (pin0Value < 126)
   pin0Value++;
}

void pin1ValueDec()
{
 if (pin1Value > -126)
   pin1Value--;
}

void pin1ValueInc()
{
 if (pin1Value < 126)
   pin1Value++;
}

void updateStateFromPin()
{
  if ((pin0Value > 0) && (pin1Value <= 0))
	sensorState = SENSOR_STATE_CLOSE;
  else
   if ((pin0Value <= 0) && (pin1Value > 0))
	 sensorState = SENSOR_STATE_OPEN;
   else
    if ((pin0Value <= 0) && (pin1Value <= 0))
	  sensorState = SENSOR_STATE_WORK;
    else
	  sensorState = SENSOR_STATE_FAULT;
}

void SensorsTimeFunc(void);

void InitSensors(void)
{
  sensorReady = SENSOR_READY_FALSE;
  sensorState = SENSOR_STATE_UNDEFINED;
  
  pin0Value = 0;
  pin1Value = 0;
  pinCounter = PIN_COUNTER_MAX;
  
  RegVirtualTimer(&SensorsTimeFunc);
  
  DDRC &= 0xe1;
  PORTC &= 0xe1;
  
  f_invalideVoltage=1;
  invalideVSensTimer=const_InvalideVSensTimer;
}

void SensorsTimeFunc(void)
{
  if (voltage < MINIMAL_VOLTAGE)
  {
   f_invalideVoltage=1;
   invalideVSensTimer=const_InvalideVSensTimer;
   
   sensorState = SENSOR_STATE_UNDEFINED;
   
   pin0Value = 0;
   pin1Value = 0;
   pinCounter = PIN_COUNTER_MAX;
  }
  else
  {
   if (f_invalideVoltage == 1)
   {
    if (invalideVSensTimer-- == 0)
    {
     f_invalideVoltage = 0;
    }
   }
   else
   {
    switch (getPin0())
    {
     case 0: pin0ValueDec();break;
     case 1: pin0ValueInc();break;
    }
	
    switch (getPin1())
    {
     case 0: pin1ValueDec();break;
     case 1: pin1ValueInc();break;
    }

    if (pinCounter != 0) {pinCounter--;}
    else
    {
     updateStateFromPin();
	 pin0Value = 0;
	 pin1Value = 0;
	 pinCounter = PIN_COUNTER_MAX;
	 sensorReady = SENSOR_READY_TRUE;
    }
   }
  }
}
