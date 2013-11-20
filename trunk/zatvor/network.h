#ifndef NETWORK_H
#define NETWORK_H

class Network{
private:
	static const unsigned int BAUD_RATE_H = 0;
	static const unsigned int BAUD_RATE_L = 3;

	static const unsigned int BUFFER_SIZE = 20;
	
	static const unsigned int DST_ADDRESS_OFFSET = 0;
	static const unsigned int SRC_ADDRESS_OFFSET = 1;
	static const unsigned int COMMAND_OFFSET = 2;
	static const unsigned int PARAMS_COUNT_OFFSET = 3;
	static const unsigned int ZATVOR_NUMBER_OFFSET = 4;
	
	static unsigned char sendBuffer[BUFFER_SIZE];
	static unsigned int sendBufferPos;
	static void initSender();

	static unsigned char recvBuffer[BUFFER_SIZE];
	static unsigned int recvBufferPos;
	static void initReceiver();
	static void stopReceiver();
	static bool fSync;
	static const int SYNC_COUNTER_MAX = 5;
	static int syncCounter;
	static void clearSyncCounter();

	static void portInit();	
	static void portToSend();
	static void portToRec();
	static unsigned char getCrc(unsigned char* data, unsigned int size);

	static unsigned char getAddress();
	static unsigned char address;
	
	static bool testRecv();
	static bool testParamsCount();
	static void processCommand();
	static int zatvorNumber;
		
public:
	static void init();

	static void send();
	static void setSendFrame(char dstAddress, char srcAddress, char command, char paramsCount, ...);
	static void startSend();

	static void recv();
	static void receiverSyncTimer();
	static bool klapanOpen;
};

#endif
