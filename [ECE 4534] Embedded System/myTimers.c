/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "timers.h"

/* include files. */
#include "vtUtilities.h"
#include "LCDtask.h"
#include "myTimers.h"
#include "I2CTaskMsgTypes.h"
/* **************************************************************** */
// WARNING: Do not print in this file -- the stack is not large enough for this task
/* **************************************************************** */

/* *********************************************************** */
// Functions for the LCD Task related timer
//
// how often the timer that sends messages to the LCD task should run
// Set the task up to run every 100 ms
#define lcdWRITE_RATE_BASE	( ( portTickType ) 150 / portTICK_RATE_MS)

// Callback function that is called by the LCDTimer
//   Sends a message to the queue that is read by the LCD Task
void LCDTimerCallback(xTimerHandle pxTimer)
{
	if (pxTimer == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		// When setting up this timer, I put the pointer to the 
		//   LCD structure as the "timer ID" so that I could access
		//   that structure here -- which I need to do to get the 
		//   address of the message queue to send to 
		vtLCDStruct *ptr = (vtLCDStruct *) pvTimerGetTimerID(pxTimer);
		// Make this non-blocking *but* be aware that if the queue is full, this routine
		// will not care, so if you care, you need to check something
		if (SendLCDTimerMsg(ptr,lcdWRITE_RATE_BASE,0) == errQUEUE_FULL) {
			// Here is where you would do something if you wanted to handle the queue being full
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}

void startTimerForLCD(vtLCDStruct *vtLCDdata) {
	if (sizeof(long) != sizeof(vtLCDStruct *)) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	xTimerHandle LCDTimerHandle = xTimerCreate((const signed char *)"LCD Timer",lcdWRITE_RATE_BASE,pdTRUE,(void *) vtLCDdata,LCDTimerCallback);
	if (LCDTimerHandle == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		if (xTimerStart(LCDTimerHandle,0) != pdPASS) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}


/* *********************************************************** */

#define tempWRITE_RATE_BASE	( ( portTickType ) 100 / portTICK_RATE_MS)

// Timer Task for requesting PIC to Update Sensor Data
void SensorUpdateTimerCallback(xTimerHandle pxTimer)
{
	if (pxTimer == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		vtSensorStruct *ptr = (vtSensorStruct *) pvTimerGetTimerID(pxTimer);
		if (SendUpdateSensorTimerMsg(ptr,tempWRITE_RATE_BASE,0) == errQUEUE_FULL) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}

void startTimerUpdateForSensor(vtSensorStruct *vtSensordata) {
		
	if (sizeof(long) != sizeof(vtSensorStruct *)) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	xTimerHandle SensorTimerHandle = xTimerCreate((const signed char *)"SensorUpdate Timer",tempWRITE_RATE_BASE,pdTRUE,(void *) vtSensordata,SensorUpdateTimerCallback);
	if (SensorTimerHandle == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		if (xTimerStart(SensorTimerHandle,0) != pdPASS) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}

}

/* *********************************************************** */
#define stateMachineWRITE_RATE_BASE	( ( portTickType ) 200 / portTICK_RATE_MS)

// Timer Task for requesting PIC to Update Sensor Data
void StateMachineTimerCallback(xTimerHandle pxTimer)
{
	if (pxTimer == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		vtSensorStruct *ptr = (vtSensorStruct *) pvTimerGetTimerID(pxTimer);
		if (SendTimerStateMachine(ptr,stateMachineWRITE_RATE_BASE,0) == errQUEUE_FULL) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}

void startTimerStateMachine(vtSensorStruct *vtSensordata) {
		
	if (sizeof(long) != sizeof(vtSensorStruct *)) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	xTimerHandle SensorTimerHandle = xTimerCreate((const signed char *)"state Timer",stateMachineWRITE_RATE_BASE,pdTRUE,(void *) vtSensordata,StateMachineTimerCallback);
	if (SensorTimerHandle == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		if (xTimerStart(SensorTimerHandle,0) != pdPASS) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}

}

/* *********************************************************** */
// Yosub	- MileStone #2 
// Timer to get Sensor Data 
#define yodaWRITE_RATE_BASE	( ( portTickType ) 200 / portTICK_RATE_MS)

void SensorGetDataTimerCallback(xTimerHandle pxTimer)
{
	if (pxTimer == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		vtSensorStruct *ptr = (vtSensorStruct *) pvTimerGetTimerID(pxTimer);
		if (GetDataTimerMsg(ptr,yodaWRITE_RATE_BASE,0) == errQUEUE_FULL) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}

void startTimerForGetData(vtSensorStruct *vtSensordata) {
		
	if (sizeof(long) != sizeof(vtSensorStruct *)) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	xTimerHandle SensorTimerHandle = xTimerCreate((const signed char *)"GetSensor Timer",yodaWRITE_RATE_BASE,pdTRUE,(void *) vtSensordata,SensorGetDataTimerCallback);
	if (SensorTimerHandle == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		if (xTimerStart(SensorTimerHandle,0) != pdPASS) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}

/* *********************************************************** */
// Yosub	- Joystick Check Timer
//
#define joystickWRITE_RATE_BASE	( ( portTickType ) 400 / portTICK_RATE_MS)

void SensorJoystickTimerCallback(xTimerHandle pxTimer)
{
	
	if (pxTimer == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		vtSensorStruct *ptr = (vtSensorStruct *) pvTimerGetTimerID(pxTimer);
		if (SendSensorJoystick(ptr,joystickWRITE_RATE_BASE,0) == errQUEUE_FULL) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}

void startTimerForJoystick(vtSensorStruct *vtSensordata) {
	
	if (sizeof(long) != sizeof(vtSensorStruct *)) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	xTimerHandle SensorTimerHandle = xTimerCreate((const signed char *)"SensorJoystick Timer",joystickWRITE_RATE_BASE,pdTRUE,(void *) vtSensordata,SensorJoystickTimerCallback);
	if (SensorTimerHandle == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		if (xTimerStart(SensorTimerHandle,0) != pdPASS) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}
