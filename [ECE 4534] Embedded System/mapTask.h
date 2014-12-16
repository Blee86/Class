#ifndef MAP_TASK_H
#define MAP_TASK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mapTask.h"
#include "vtUtilities.h"
#include "vtI2C.h"
#include "i2cSensor.h"
#include "I2CTaskMsgTypes.h"
#include "conductor.h"
#include "LCDtask.h"
#include "GLCD.h"
/*
typedef struct node {
    uint8_t x;
    uint8_t y;
    struct node* next;
} node;

typedef struct Queue {
    struct node *front;
    struct node *rear;
    uint16_t size;
} Queue;
*/

typedef struct __MapStruct {
	vtI2CStruct *dev;
	vtLCDStruct *lcdData;
	xQueueHandle inQ;
} vtMapStruct;
#endif