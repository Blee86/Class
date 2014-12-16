#include "simulation.h"

unsigned char map[100][100];
unsigned char mapData[6];
uint8_t initialMap[] = {10
,-12, -32
,  0, -32
,  0,  -7
,  5,  -7
,  5, -14
, 25, -14
, 25,   0
,  0,   0
,  0,  47
,-12,  47
,  2
, -8,  43
,  0,   1
, 1,   0
, 15,  -7
,  1,   0
,  1,   0
, -8, -24};
unsigned char fixedDirection[6];
int x_coordinate, y_coordinate;
int direction = 0;	// 0 - N, 1- E, 2 - S, 3 - W

void initMap() {
	uint8_t corners = initialMap[0];
	
	uint8_t x,y;
	uint16_t i;
	
	uint16_t iterate = 1 + (corners * 2);
	for ( i = 1; i < iterate; i++ ) {
		// Check even or odd
		if ( (i&1) == 1 ) {	// Odd
			x = initialMap[i];
			y = initialMap[i+1];
			
			x = x + 50;
			y = y + 50;
			
			map[x][y] = 1;
			i++;
		}
	}
}

void copyMap(unsigned char *buf) {
	int i=0;
	for (i=0; i< 6;i++ ) {
		buf[i] = mapData[i];			
	}
}

void updateSensor() {
	unsigned char position[6];
	
	switch(direction){
	case 0:	//North
		mapData[0] = 30 - y_coordinate;
		mapData[1] = 30 - x_coordinate;
		mapData[2] = 30 - x_coordinate;
		mapData[3] = y_coordinate;
		mapData[4] = x_coordinate;
		mapData[5] = x_coordinate;
		break;
	case 1:
		mapData[0] = 30 - x_coordinate;
		mapData[1] = y_coordinate;
		mapData[2] = y_coordinate;
		mapData[3] = x_coordinate;
		mapData[4] = 30 - y_coordinate;
		mapData[5] = 30 - y_coordinate;
		break;
	case 2:
		mapData[0] = y_coordinate;
		mapData[1] = x_coordinate;
		mapData[2] = x_coordinate;
		mapData[3] = 30 - y_coordinate;
		mapData[4] = 30 - x_coordinate;
		mapData[5] = 30 - x_coordinate;
		break;
	case 3:
		mapData[0] = x_coordinate;
		mapData[1] = 30 - y_coordinate;
		mapData[2] = 30 - y_coordinate;
		mapData[3] = 30 - x_coordinate;
		mapData[4] = y_coordinate;
		mapData[5] = y_coordinate;
		break;
	default:
		break;
	}
}

void updateFixed() {
		fixedDirection[0] = 30 - y_coordinate;
		fixedDirection[1] = 30 - x_coordinate;
		fixedDirection[2] = 30 - x_coordinate;
		fixedDirection[3] = y_coordinate;
		fixedDirection[4] = x_coordinate;
		fixedDirection[5] = x_coordinate;

}

void copyFixedMap(unsigned char *buf) {
	int i=0;
	for (i=0; i< 6;i++ ) {
		buf[i] = fixedDirection[i];			
	}
}

void moveForward() {
	if ( direction == 0) {
		y_coordinate++;
	}
	else if ( direction == 1) {
		x_coordinate++;
	}
	else if ( direction == 2) {
		y_coordinate--;
	}
	else {
		x_coordinate--;
	}

}

void turnLeft() {
	
	if (direction == 0) direction = 3;
	else if(direction == 1) direction = 0;
	else if(direction == 2) direction = 1;
	else direction = 2;

}

void turnRight() {
	if (direction == 0) direction = 1;
	else if(direction == 1) direction = 2;
	else if(direction == 2) direction = 3;
	else direction = 0;
}
