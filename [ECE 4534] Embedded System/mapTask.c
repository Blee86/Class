#include "mapTask.h"

#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"

#define baseStack 2
#define mapSTACK_SIZE	(baseStack*configMINIMAL_STACK_SIZE)

#define vtMapQueueLen 20
static portTASK_FUNCTION_PROTO( vMapUpdateTask, pvParameters );

typedef struct __vtSensorMsg {
	uint8_t msgType;
	uint8_t	length;	 // Length of the message to be printed
	uint8_t buf[vtSensorMaxLen+1]; // On the way in, message to be sent, on the way out, message received (if any)
} vtSensorMsg;

void vStartMapTask(vtSensorStruct *params,unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c,vtLCDStruct *lcd)
{
	if ((params->inQ = xQueueCreate(vtMapQueueLen,sizeof(vtSensorMsg))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	/* Start the task */
	portBASE_TYPE retval;
	params->dev = i2c;
	params->lcdData = lcd;
	if ((retval = xTaskCreate( vMapUpdateTask, ( signed char * ) "MapTask", mapSTACK_SIZE, (void *) params, uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

// opt  = 0 - even(x)  1 - odd(y)
static int min(char *array, int length, uint8_t opt) {
    uint16_t index = 0;
    int temp = array[index];
    
    uint8_t i = 0;
    
    if ( opt == 0) {
        for (i = 0; i < length; i +=2) {
            if (array[i] < temp)
                temp = array[i];
        }
    }
    else {
        for (i = 1; i < length; i +=2) {
            if (array[i] < temp)
                temp = array[i];
        }
    }
    return temp;
}

static int max(char *array, int length, uint8_t opt) {
    uint16_t index = 0;
    int temp = array[index];
    
    uint8_t i = 0;
    
    if ( opt == 0) {
        for (i = 0; i < length ; i +=2) {
            if (array[i] > temp)
                temp = array[i];
        }
    }
    else {
        for (i = 1; i < length ; i +=2) {
            if (array[i] > temp)
                temp = array[i];
        }
    }
    return temp;
}

// Negative to Postive.
static void updateMapArray(char *array, int length, char x_inc, char y_inc) {
    uint8_t i = 0;
    
    for( i = 0; i < length; i++) {
        if ( i%2 == 0) { //X
            array[i] += x_inc;
        }
        else {
            array[i] += y_inc;
        }
    }
}

// Initialize 2d array
static void Init_map(uint8_t **map, char *map_array, int length) {
    int i = 0;
    uint8_t x, y, x_prev, y_prev;
    char x_temp, y_temp;
    
    x_prev = map_array[0];
    y_prev = map_array[1];
    
    for ( i = 2; i < length; i+=2) {
        x = map_array[i];
        y = map_array[i+1];
        
        if ( (x_prev - x) != 0 ) {
            if ( x_prev > x ) {
                uint8_t x_length = abs(x_prev - x);
                uint8_t k = 0;
                for (k = 0; k < x_length; k++) {
                    map[x_prev-k][y] = 1;
                    printf("%d, %d = 1", x_prev-k, y);
                }
            }
            else {
                uint8_t x_length = x_prev - x;
                uint8_t k = 0;
                for (k = 0; k < x_length; k++) {
                    map[x_prev + k][y] = 1;
                    printf("%d, %d = 1", x_prev+k, y);
                }
            }
        }
        else if ( (y_prev - y) != 0) {
            if ( y_prev > y ) {
                uint8_t y_length = abs(y_prev - y);
                uint8_t k = 0;
                for (k = 0; k < y_length; k++) {
                    map[x][y] = 1;
                    printf("%d, %d = 1", x_prev-k, y);
                }
            }
            else {
                uint8_t x_length = x_prev - x;
                uint8_t k = 0;
                for (k = 0; k < x_length; k++) {
                    map[x_prev + k][y] = 1;
                    printf("%d, %d = 1", x_prev+k, y);
                }
            }
        }
    }
    
}


static portTASK_FUNCTION( vMapUpdateTask, pvParameters) {
    /* Map initializing */
    char map_array[20] = {-12, -32, 0, -32, 0, -7, 5, -7, 5, -14, 25, -14, 25, 0, 0, 0, 0, 47, -12, 47};
    uint8_t length = 20;
    char x_inc = abs(min(map_array, 20, 0));
    char y_inc = abs(min(map_array, 20, 1));
    
    updateMapArray(map_array, 20, x_inc, y_inc);
    int i = 0;
    for (i=0; i < 20; i +=2 ){
        printf("(%d, %d) \n", map_array[i], map_array[i+1]);
    }
    
    uint8_t x_length = max(map_array, 20, 0);
    uint8_t y_length = max(map_array, 20, 1);
   
    // Map
    uint8_t map[x_length+1][y_length+1];
    
    // Set Initial value
    // All set to zero
    int j =0;
    
    for ( i =0; i < x_length; i++) {
        for ( j=0; j < y_length; j++ ){
            map[i][j] = 0;
        }
    }
    
    // Connect points
    uint8_t x,y,x1, y1, x2, y2;
    int distance;
    
    for (j=0; j < length; j += 2) {
        if ( j == 18) {
            x1 = map_array[j];
            y1 = map_array[j+1];
            
            x2 = map_array[0];
            y2 = map_array[1];
        }
        else{
            x1 = map_array[j];
            y1 = map_array[j+1];
            x2 = map_array[j+2];
            y2 = map_array[j+3];
        }
        
        if (x1 == x2) {
            if (y1 < y2) {
                distance = y2 - y1;
                x = x1;
                y = y1;
                for (i = 0; i < distance; i++) {
                    map[x][y+i] = 1;
                }
            }
            else {
                distance = y1 - y2;
                x = x1;
                y = y2;
                for ( i = 0; i < distance; i++) {
                    map[x][y+i] = 1;
                }
            }
        }
        else if (y1 == y2) {
            if (x1 < x2) {
                distance = x2 - x1;
                x = x1;
                y = y1;
                for ( i = 0; i < distance; i++) {
                    map[x+i][y] = 1;
                }
            }
            else {
                distance = x1 - x2;
                x = x2;
                y = y1;
                for ( i = 0; i < distance;  i++) {
                    map[x+i][y] = 1;
                }
            }
        }
    }
    
	/* Actual Task Part */

	vtSensorStruct *param = (vtSensorStruct *) pvParameters;
	vtI2CStruct *devPtr = param->dev;
	vtLCDStruct *lcdData = param->lcdData;
	vtSensorMsg msgBuffer;
	char lcdBuffer[30];
	
	for(;;)
	{
		if (xQueueReceive(param->inQ,(void *) &msgBuffer,portMAX_DELAY) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}	
	}
}