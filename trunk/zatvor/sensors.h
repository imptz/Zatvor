void InitSensors(void);

enum SENSOR_STATE
{
 SENSOR_STATE_UNDEFINED = 0,  
 SENSOR_STATE_OPEN = 1,  
 SENSOR_STATE_CLOSE = 2,  
 SENSOR_STATE_WORK = 3,  
 SENSOR_STATE_FAULT = 4  
};

enum SENSOR_STATE getSensorState(void);

enum SENSOR_READY
{
 SENSOR_READY_TRUE = 0,  
 SENSOR_READY_FALSE = 1
};

enum SENSOR_READY getSensorReady(void);

unsigned char getButtonLine(void);

