#ifndef _MY_TIMERS_H
#define _MY_TIMERS_H
#include "lcdTask.h"
#include "i2cSensor.h"
void startTimerForLCD(vtLCDStruct *vtLCDdata);
void startTimerUpdateForSensor(vtSensorStruct *vtSensordata);
void startTimerForGetData(vtSensorStruct *vtSensordata);
void startExtInteruptCheck(vtSensorStruct *vtSensordata);
void startTimerForStatus(vtSensorStruct *vtSensordata);
#endif