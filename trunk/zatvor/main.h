//#define BUTTONS 
//#define ZATVOR_OLD
#define ZATVOR_2

void SetAvarIndication(char state);
void SetWorkIndication(char state);
//extern unsigned char curProg;

#define initMaxCurrent0 60
#define const_countTimeMaxCurrent0 700
#define initMaxCurrent1 100
#define const_countTimeMaxCurrent1 200
#define const_driveStopCounter 250

void delay_mks_10(int time);
void delay_mks_100(int time);
void delay_mks_1000(int time);
void delay(int n);
void initDelay(void);
#define delay_ms delay_mks_1000

extern void SetOuts(unsigned char);
extern char outsState;

void zatvorOpen();
void zatvorClose();
void zatvorStop();
