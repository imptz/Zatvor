typedef void (*TPointToHandlerADCInterrupt)(void);
extern void RegChannel(TPointToHandlerADCInterrupt fpoint,unsigned char channel);
extern void InitADC(void);
#define ADCChannel_0 0
#define ADCChannel_6 6
#define ADCChannel_7 7
