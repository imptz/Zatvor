#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdarg.h>

#include "main.h"
#include "network.h"
#include "timers.h"

#include "main.h"
#include "drive.h"
#include "but.h"
#include "sensors.h"
#include "lamp.h"


// ==========================================================

//void dbgOn(){
	//DDRD |= 0x60;
	//PORTD &= 0x9f;
	//PORTD |= 0x60;
//}
//
//void dbgOff(){
	//DDRD |= 0x60;
	//PORTD &= 0x9f;
	//PORTD |= 0x00;
//}
//
//const unsigned int OUT_VAL_MAX = 10;
//unsigned int outVal = 0;
//unsigned int setOutVal = 0;
//void setDebugVal(unsigned int val){
	//setOutVal = val;
//}
//
//void debugOutTimer(){
	//static unsigned int count = 0;
	//if(outVal++ < setOutVal){
		//dbgOff();
	//}else
		//if(outVal++ < OUT_VAL_MAX){
			//dbgOn();
		//}else			
			//outVal = 0;
//}
//
// ==========================================================

unsigned char Network::sendBuffer[Network::BUFFER_SIZE];
unsigned int Network::sendBufferPos;

unsigned char Network::recvBuffer[Network::BUFFER_SIZE];
unsigned int Network::recvBufferPos;
bool Network::fSync;
int Network::syncCounter;

unsigned char Network::address;

int Network::zatvorNumber;
bool Network::klapanOpen;

void networkTimerFunc(){
	//debugOutTimer();
	Network::receiverSyncTimer();
}

void Network::portInit(){
	DDRC |= 0x20;
	PORTC &= 0xdf;
}
	
void Network::portToSend(){
	PORTC |= 0x20;
}
	
void Network::portToRec(){
	PORTC &= 0xdf;
}

ISR(USART_TXC_vect){
	Network::send();
}

ISR(USART_RXC_vect){
	Network::recv();
}

unsigned char Network::getAddress(){
	DDRB &= 0xc7;
	PORTB |= 0x38;
	DDRD &= 0xe3;
	PORTD |= 0x1c;
	
	delay_mks_10(10);
	
	unsigned char _address = ((~PIND) & (1 << 2)) >> 1;
	_address |= (((~PIND) & (1 << 3)) >> 1);
	_address |= (((~PIND) & (1 << 4)) >> 1);
	_address |= (((~PINB) & (1 << 3)) >> 3);
	_address |= (((~PINB) & (1 << 5)) >> 1);
	_address |= (((~PINB) & (1 << 4)) << 1);	

 	return _address;
}

unsigned char testAddress;
void Network::init(){
	klapanOpen = false;
	zatvorNumber = 0;
#ifdef ZATVOR_2
	zatvorNumber = 1;
#endif	
	portInit();
	
	address = getAddress();
		
	UBRRH = BAUD_RATE_H;
	UBRRL = BAUD_RATE_L;
	
	UCSRC = ((1 << URSEL) | (1 << UCSZ1) |  (1 << UCSZ0));
	
	initSender();
	initReceiver();
	portToRec();		
	RegVirtualTimer(&networkTimerFunc);
}

void Network::initSender(){
	sendBufferPos = 0;
	for(unsigned int i = 0; i < BUFFER_SIZE; ++i){
		sendBuffer[i] = 0;
	}
}

void Network::send(){
	if(++sendBufferPos < sendBuffer[PARAMS_COUNT_OFFSET] + 5)
		UDR = sendBuffer[sendBufferPos];
	else{
		portToRec();
		UCSRB &= ~((1 << TXCIE) | (1 << TXEN));
		initReceiver();
	}
}

void Network::setSendFrame(char dstAddress, char srcAddress, char command, char paramsCount, ...){
	va_list funcParams;
	va_start(funcParams, paramsCount);
	sendBuffer[0] = dstAddress;
	sendBuffer[1] = srcAddress;
	sendBuffer[2] = command;
	sendBuffer[3] = paramsCount;
	for(unsigned i = 4; i < 4 + paramsCount; ++i){
		sendBuffer[i] = va_arg(funcParams, int);
	}
	
	va_end(funcParams);
	
	sendBuffer[4 + paramsCount] = getCrc(sendBuffer, 4 + paramsCount);
}

void Network::startSend(){
	stopReceiver();
	portToSend();
	
	UCSRA |= (1 << TXC);
	UCSRB |= ((1 << TXCIE) | (1 << TXEN));
	
	delay_mks_10(10);
	sendBufferPos = 0;
	UDR = sendBuffer[0];
}

unsigned char Network::getCrc(unsigned char* data, unsigned int size){
	unsigned char crc = 0;
	
	for(unsigned int i = 0; i < size; ++i){
		crc += data[i];
	}	
	
	return crc;
}

void Network::initReceiver(){
	recvBufferPos = 0;
	for(unsigned int i = 0; i < BUFFER_SIZE; ++i){
		recvBuffer[i] = 0;
	}
	
	fSync = false;
	syncCounter = 0;
	
	UCSRA |= (1 << RXC);
	UCSRB |= ((1 << RXCIE) | (1 << RXEN));
}

void Network::stopReceiver(){
	UCSRB &= ~((1 << RXCIE) | (1 << RXEN));
}	
	
void Network::clearSyncCounter(){
	syncCounter = 0;
}

void Network::receiverSyncTimer(){
	if(++syncCounter >= SYNC_COUNTER_MAX){
		fSync = true;
		clearSyncCounter();
	}
}

void Network::recv(){
	clearSyncCounter();
	
	unsigned char status = UCSRA;
	
	if ((status & (1 << FE | 1 << DOR)) != 0){
		status = UDR;
		initReceiver();
	}else{
		recvBuffer[recvBufferPos] = UDR;
		
		if(!fSync)
			return;
			
		if(recvBufferPos == PARAMS_COUNT_OFFSET){
			if (!testParamsCount()){
				//setDebugVal(1);
				initReceiver();
				return;
			}					
		}else
			if(recvBufferPos == recvBuffer[PARAMS_COUNT_OFFSET] + 4){
				if (getCrc(recvBuffer, recvBuffer[PARAMS_COUNT_OFFSET] + 4) != 
						recvBuffer[recvBuffer[PARAMS_COUNT_OFFSET] + 4]){
					initReceiver();
					//setDebugVal(2);
				}else if (!testRecv()){
					initReceiver();
					return;
				}
									
				processCommand();	
				recvBufferPos = 0;	
				return;
			}				
			++recvBufferPos;
	}	
}

bool Network::testParamsCount(){
	return recvBuffer[PARAMS_COUNT_OFFSET] <= 15;
}

bool Network::testRecv(){	
	bool result = true;
	
	if (recvBuffer[DST_ADDRESS_OFFSET] != address)
		return true;
	
	switch(recvBuffer[COMMAND_OFFSET]){
		case 9:
		case 10:
		case 11:
		case 158:
		case 159:
			break;
		default:
			result = false;
			//setDebugVal(3);
			break;
	}

	if (!testParamsCount()){
		result = false;
		//setDebugVal(4);
	}		
		
	switch(recvBuffer[SRC_ADDRESS_OFFSET]){
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
			break;
		default:
			result = false;
			//setDebugVal(5);
			break;
	}	
	
	return result;
}

void Network::processCommand(){
	if (recvBuffer[DST_ADDRESS_OFFSET] != address)
		return;

	unsigned char temp = 0, temp1 = 0;
	
	switch (recvBuffer[COMMAND_OFFSET]){
		case 9:
#ifdef ZATVOR_OLD		
			zatvorOpen();
			setSendFrame(recvBuffer[SRC_ADDRESS_OFFSET], address, 120, 1, 9);
			startSend();
#else
			if (zatvorNumber == recvBuffer[ZATVOR_NUMBER_OFFSET]){
				zatvorOpen();
				setSendFrame(recvBuffer[SRC_ADDRESS_OFFSET], address, 120, 1, 9);
				startSend();
			}
#endif			
			break;
		case 10:
#ifdef ZATVOR_OLD
			zatvorClose();
			setSendFrame(recvBuffer[SRC_ADDRESS_OFFSET], address, 120, 1, 10);
			startSend();
#else
			if (zatvorNumber == recvBuffer[ZATVOR_NUMBER_OFFSET]){
				zatvorClose();
				setSendFrame(recvBuffer[SRC_ADDRESS_OFFSET], address, 120, 1, 10);
				startSend();
			}
#endif		
			break;
		case 11:
#ifdef ZATVOR_OLD
			zatvorStop();
			setSendFrame(recvBuffer[SRC_ADDRESS_OFFSET], address, 120, 1, 11);
			startSend();
#else
			if (zatvorNumber == recvBuffer[ZATVOR_NUMBER_OFFSET]){
				zatvorStop();
				setSendFrame(recvBuffer[SRC_ADDRESS_OFFSET], address, 120, 1, 11);
				startSend();
			}
#endif			
			break;
		case 159:
#ifndef ZATVOR_2
			SetOuts(recvBuffer[PARAMS_COUNT_OFFSET + 1]);
			setSendFrame(recvBuffer[SRC_ADDRESS_OFFSET], address, 120, 1, 159);
			startSend();
#endif			
			break;
		case 158:
			temp = 0;
			switch (getSensorState()){
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
	 
			temp|= f_excessCurrentCache;
			f_excessCurrentCache = 0;
			if (driveDirect == 1){
				temp1 = current[1];
			}else if (driveDirect == 2){
				temp1 = current[0];
			}else{
				temp1 = 0;
			}
			
			if(klapanOpen)
				temp |= 0x40;
#ifndef ZATVOR_OLD				
				if (zatvorNumber == recvBuffer[ZATVOR_NUMBER_OFFSET]){
					setSendFrame(recvBuffer[SRC_ADDRESS_OFFSET], address, 120, 5, 158, temp, temp1, zatvorNumber, getButtonLine());
					startSend();
				}					
#else
				setSendFrame(recvBuffer[SRC_ADDRESS_OFFSET], address, 120, 5, 158, temp, temp1, 0, getButtonLine());
				startSend();
#endif			
			break;
	}
}