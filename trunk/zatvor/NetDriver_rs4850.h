typedef void (*TPointer_HandlerRecPackage)(void);
typedef void (*TPointer_SetUpPortUartFunc)(void);
typedef void (*TPointer_RecToSendFunc)(void);
typedef void (*TPointer_SendToRecFunc)(void);


// необходимо инициализировать
extern unsigned char rs485_deviceAddress;
extern unsigned char rs485_uartSpeedh;
extern unsigned char rs485_uartSpeedl;
extern unsigned char rs485_f_sending;
// ----------------------------

extern unsigned char rs485_receiveBuf[21];
extern unsigned char rs485_sendBuf[21];

extern void rs485_InitNetDriver_RS485(char lDeviceAddress,char ubh,char ubl, TPointer_HandlerRecPackage lPointer_HandlerRecPackage,
	 				  	 TPointer_SetUpPortUartFunc lPointer_SetUpPortUartFunc,
						 TPointer_RecToSendFunc lPointer_RecToSendFunc,
						 TPointer_SendToRecFunc lPointer_SendToRecFunc);
extern void rs485_StartSend(void);
extern void rs485_FillPackage(char da,char sa,char com,char npar, ...);
extern void rs485_FillPackage_1(char da,char sa,char com,char npar, ...);
#define FRAMING_ERROR (0x10)
#define DATA_OVERRUN  (0x08)

void rs485_InitNetwork(void);

extern unsigned char rs485_bVal[4];
#define rs485_il rs485_bVal[0]
#define rs485_ir rs485_bVal[1]
#define rs485_iu rs485_bVal[3]
#define rs485_id rs485_bVal[2]

