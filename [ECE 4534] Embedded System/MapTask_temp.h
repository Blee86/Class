#ifndef MAP_TASK_H
#define MAP_TASK_H

#include "vtI2C.h"
#inlcude "lcdTask.h"
#include "quadtree.h"
#define mapSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)


typedef struct vtMapStruct {
	vtI2CStruct *dev;
	vtSensorStruct *sensorData;
	xQueueHandle inQ;
} vtMapStruct;

typedef struct vtMapMsg {
	uint8_t msgType;
	uint8_t length;
	uint8_t buf[vtSensorMaxLen+1];
}

portBASE_TYPE SendMapValueMsg(vtMapStruct *mapData, portTickType ticksElapsed, portTickType ticksToBlock) 
{
	if ( mapData == NULL ) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	
	vtMapMsg mapBuffer;
	mapBuffer.length = sizeof(ticksElapsed);
	
	if (mapBuffer.length > vtSensorMaxLen) {
		VT_HANDLE_FATAL_ERROR(mapBuffer.length);
	}
	
	memcpy(mapBuffer.buf, (char *)&ticksElapsed, sizeof(ticksElapsed));
	
	
	// Decide Something here!!!!
	//mapBuffer.msgType = 1
	
	return(xQueueSend(mapData->inQ, (void *) (&mapBuffer), ticksToBlock));
}


static portTASK_FUNCTION_PROTO( vMapUpdateTask, pvParameters );


void vStartMapTask(vtMapStruct *params, unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c, vtSensorStruct *sensor)
{
	portBASE_TYPE retval;
	params->dev = i2c;
	params->sensorData = sensor;

	if ((retval = xTaskCreate( vConductorUpdateTask, ( signed char * ) "Map", mapSTACK_SIZE, (void *) params, uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}



static portTASK_FUNCTION( vMapUpdateTask, pvParameters )
{
	vtSensorStruct *param = (vtSensorStruct *) pvParameters;
	vtI2CStruct *devPtr = param->dev;
	vtLCDStruct *lcdData = param->lcdData;
	
	
	vtMapMsg msgBuffer;
	
	for (;;)
	{
		if (xQueueReceive(param->inQ, (void *) &msgBuffer, portMAX_DELAY) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	
		switch(getMsgType(&msgBuffer)) {
		case MapSensorData:
		
			break;
			
		default:
			VT_HANDLE_FATAL_ERROR(0);
			break;
			
		}
	}

}