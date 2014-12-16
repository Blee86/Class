#ifndef SIMULATION_H
#define SIMULATION_H
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"

/*
	Simulation
	- For now It's using a 2 dimensional Array
*/


void initMap();
void updateSensor();
void updateFixed();
void moveForward();
void turnLeft();
void turnRight();
void copyMap(unsigned char *buf);
void copyFixedMap(unsigned char *buf);
#endif