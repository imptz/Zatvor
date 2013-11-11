extern void InitVirtualTimers(void);
typedef void (*TPointToHandlerVirtualTimers)(void);
extern void RegVirtualTimer(TPointToHandlerVirtualTimers fpoint); 
 

