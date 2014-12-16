#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"

/* include files. */
#include "vtUtilities.h"
#include "vtI2C.h"
#include "LCDtask.h"
#include "i2cSensor.h"
#include "I2CTaskMsgTypes.h"
#include "mapping.h"

#include "joystick.h"
/* *********************************************** */
// definitions and data structures that are private to this file
// Length of the queue to this task
#define vtSensorQueueLen 20 
// actual data structure that is sent in a message
typedef struct __vtSensorMsg {
	uint8_t msgType;
	uint8_t	length;	 // Length of the message to be printed
	uint8_t buf[vtSensorMaxLen+1]; // On the way in, message to be sent, on the way out, message received (if any)
} vtSensorMsg;

// I have set this to a large stack size because of (a) using printf() and (b) the depth of function calls
//   for some of the i2c operations	-- almost certainly too large, see LCDTask.c for details on how to check the size
#define baseStack 3
#if PRINTF_VERSION == 1
#define i2cSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)
#else
#define i2cSTACK_SIZE		(baseStack*configMINIMAL_STACK_SIZE)
#endif

// end of defs
/* *********************************************** */

/* The i2cTemp task. */
static portTASK_FUNCTION_PROTO( vi2cSensorUpdateTask, pvParameters );

/*-----------------------------------------------------------*/
// Public API
void vStarti2cSensorTask(vtSensorStruct *params,unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c,vtLCDStruct *lcd)
{
	// Create the queue that will be used to talk to this task
	if ((params->inQ = xQueueCreate(vtSensorQueueLen,sizeof(vtSensorMsg))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	/* Start the task */
	portBASE_TYPE retval;
	params->dev = i2c;
	params->lcdData = lcd;
	if ((retval = xTaskCreate( vi2cSensorUpdateTask, ( signed char * ) "i2cSensor", i2cSTACK_SIZE, (void *) params, uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

// 
portBASE_TYPE GetDataTimerMsg(vtSensorStruct *sensorData,portTickType ticksElapsed,portTickType ticksToBlock)
{
	if (sensorData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtSensorMsg sensorBuffer;
	sensorBuffer.length = sizeof(ticksElapsed);
	if (sensorBuffer.length > vtSensorMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(sensorBuffer.length);
	}
	memcpy(sensorBuffer.buf,(char *)&ticksElapsed,sizeof(ticksElapsed));
	sensorBuffer.msgType = TimerReadSensor;
	return(xQueueSend(sensorData->inQ,(void *) (&sensorBuffer),ticksToBlock));
}

// Update Sensor Value Msg to the Sensor Task
portBASE_TYPE SendUpdateSensorTimerMsg(vtSensorStruct *sensorData,portTickType ticksElapsed,portTickType ticksToBlock)
{
	if (sensorData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtSensorMsg sensorBuffer;
	sensorBuffer.length = sizeof(ticksElapsed);
	if (sensorBuffer.length > vtSensorMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(sensorBuffer.length);
	}
	memcpy(sensorBuffer.buf,(char *)&ticksElapsed,sizeof(ticksElapsed));
	sensorBuffer.msgType = TimerUpdateSensor;
	return(xQueueSend(sensorData->inQ,(void *) (&sensorBuffer),ticksToBlock));
}

// Timer State Machine
 // Update Sensor Value Msg to the Sensor Task
portBASE_TYPE SendTimerStateMachine(vtSensorStruct *sensorData,portTickType ticksElapsed,portTickType ticksToBlock)
{
	if (sensorData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtSensorMsg sensorBuffer;
	sensorBuffer.length = sizeof(ticksElapsed);
	if (sensorBuffer.length > vtSensorMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(sensorBuffer.length);
	}
	memcpy(sensorBuffer.buf,(char *)&ticksElapsed,sizeof(ticksElapsed));
	sensorBuffer.msgType = TimerStateMachine;
	return(xQueueSend(sensorData->inQ,(void *) (&sensorBuffer),ticksToBlock));
}
// Any types of sensor Msg to the Sensor Task
portBASE_TYPE SendSensorValueMsg(vtSensorStruct *sensorData,uint8_t msgType,uint8_t* value,uint8_t rxLen, portTickType ticksToBlock)
{
	vtSensorMsg sensorBuffer;

	if (sensorData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	sensorBuffer.length = rxLen;
	if (sensorBuffer.length > vtSensorMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(sensorBuffer.length);
	}
	memcpy(sensorBuffer.buf,value,rxLen);
	sensorBuffer.msgType = msgType;
	return(xQueueSend(sensorData->inQ,(void *) (&sensorBuffer),ticksToBlock));
}

// Temp - Send Joystick Sginal
portBASE_TYPE SendSensorJoystick(vtSensorStruct *sensorData,portTickType ticksElapsed,portTickType ticksToBlock)
{
	if (sensorData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtSensorMsg sensorBuffer;
	sensorBuffer.length = sizeof(ticksElapsed);
	if (sensorBuffer.length > vtSensorMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(sensorBuffer.length);
	}
	memcpy(sensorBuffer.buf,(char *)&ticksElapsed,sizeof(ticksElapsed));
	sensorBuffer.msgType = I2CJoystick;
	return(xQueueSend(sensorData->inQ,(void *) (&sensorBuffer),ticksToBlock));
}
// Temp Command by Button
portBASE_TYPE SendButtonSignal(vtSensorStruct *sensorData, portTickType ticksToBlock)
{
	vtSensorMsg sensorBuffer;

	if (sensorData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	
	sensorBuffer.msgType = I2CButtonITR;
	return(xQueueSend(sensorData->inQ,(void *) (&sensorBuffer),ticksToBlock));
}
// End of Public API
/*-----------------------------------------------------------*/
int getMsgType(vtSensorMsg *Buffer)
{
	return(Buffer->msgType);
}

uint8_t getValue(vtSensorMsg *Buffer)
{
	uint8_t *ptr = (uint8_t *) Buffer->buf;
	return(*ptr);
}
	 
// Direction;
uint8_t find_closestWall(uint8_t* sensors) 
{
	uint8_t index = 0;
	uint8_t small = sensors[0];
	uint8_t i = 0;
	for (i = 0; i < 5; i++ ) {
		if ( sensors[i] < small ) {
			small = sensors[i];
			index = i;
		}
	}	
	return i;
}

// opt  = 0 - even(x)  1 - odd(y)
static int minimum(char *array, int length, uint8_t opt) {
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

static int maximum(char *array, int length, uint8_t opt) {
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
                }
            }
            else {
                uint8_t x_length = x_prev - x;
                uint8_t k = 0;
                for (k = 0; k < x_length; k++) {
                    map[x_prev + k][y] = 1;
                }
            }
        }
        else if ( (y_prev - y) != 0) {
            if ( y_prev > y ) {
                uint8_t y_length = abs(y_prev - y);
                uint8_t k = 0;
                for (k = 0; k < y_length; k++) {
                    map[x][y] = 1;
                }
            }
            else {
                uint8_t x_length = x_prev - x;
                uint8_t k = 0;
                for (k = 0; k < x_length; k++) {
                    map[x_prev + k][y] = 1;
                }
            }
        }
    }   
}

	/* I2C Command */
	const uint8_t i2cArmInit = 0xFF;
	const uint8_t i2cRequestUpateSensor = 0xA5;		// Update
	const uint8_t i2cSensorRead = 0xA0;

	/* Rover Command */
	const uint8_t i2cCommandMF = 0xA6; // Move Forward
	const uint8_t i2cCommandTL = 0xA7; // Turn Left (90 degree turn)
	const uint8_t i2cCommandMB = 0xA8; // Backward
	const uint8_t i2cCommandTR = 0xA9; // Turn Right (90 degree turn)
	const uint8_t i2cCommandST = 0xAA; // Stop
	const uint8_t i2cCommandTLS = 0xC0; // Turn Left slightly
	const uint8_t i2cCommandTRS = 0xC1; // turn Right Slightly


	// State Machine to move along the wall
	const uint8_t start 			= 0;
	const uint8_t idle				= 1;		// Check Sensors
	const uint8_t check_sensor		= 2;
	const uint8_t move				= 3;
	const uint8_t stop				= 4;
	const uint8_t turnLeft			= 5;
	const uint8_t wait_for_turn		= 6;
	const uint8_t adjust			= 7;
	const uint8_t adjust_wait		= 8;
	const uint8_t parallel			= 9;
	const uint8_t check_sensor2 	= 10;
	const uint8_t turnRight_s		= 11;
	const uint8_t turnLeft_s		= 12;
	const uint8_t moveFW			= 13;
	const uint8_t parallel_wait		= 14;
	const uint8_t stop2				= 15;
	const uint8_t stop3				= 16;
	const uint8_t turnRight			= 17;
	const uint8_t moveMF_after_right = 18;
	const uint8_t wait_for_turn2	= 19;
	const uint8_t moveForward		= 20;
									  	
static portTASK_FUNCTION( vi2cSensorUpdateTask, pvParameters )
{	
	
	// Get the parameters
	vtSensorStruct *param = (vtSensorStruct *) pvParameters;
	// Get the I2C device pointer
	vtI2CStruct *devPtr = param->dev;
	// Get the LCD information pointer
	vtLCDStruct *lcdData = param->lcdData;

	// String buffer for printing
	char lcdBuffer[30];

	// Two bytes array for Graphing
	// Buffer for receiving messages
	vtSensorMsg msgBuffer;
	
	uint8_t currentState = 0;

	// localization
	uint8_t counter = 0; 
	uint8_t mv_counter = 0;
	uint8_t sdistance = 0;

	uint8_t RBsensor, RFsensor, LBsensor, LFsensor, FrontSensor, sensor_ensured;
	uint8_t RFsensor_prev, LFsensor_prev, FrontSensor_prev;
	
	unsigned char roverCommand[2];

	uint8_t stateMachineRunning = 1;
	// temp
	uint8_t ret;
	uint8_t temp_prev;
	uint8_t FrontSensor_diff_prev;
	uint8_t roverState;
	uint8_t stop_flag = 0;
	uint8_t left_flag = 0;
	currentState = start;
	roverState = 0;
    // EnQueue a request to the PIC board to update sensor data
	if (vtI2CEnQ(devPtr,I2CARMInit,0x4F,sizeof(&i2cArmInit),&i2cArmInit,0) != pdTRUE) {
		VT_HANDLE_FATAL_ERROR(0);					  
	}
	
	for(;;)
	{
		// Dequeue
		if (xQueueReceive(param->inQ,(void *) &msgBuffer,portMAX_DELAY) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}	

		switch(getMsgType(&msgBuffer)) {
		
			case TimerUpdateSensor:{       // From Timer. Ask PIC to update Sensor Data
				vtLEDToggle(1);
				// Update Sensor data 											// 0xAA
				roverCommand[0] = i2cRequestUpateSensor;
				roverCommand[1] = 0x00;
				if (vtI2CEnQ(devPtr,TimerUpdateSensorRequested,0x4F,sizeof(&i2cRequestUpateSensor),&i2cRequestUpateSensor, 0) != pdTRUE) {
					VT_HANDLE_FATAL_ERROR(0);
				}
				break;		
			}
			case TimerReadSensor: {
				vtLEDToggle(2);
				// Get the sensor data 	From ARM side PIC board					// 0xA5
				if (vtI2CEnQ(devPtr,I2CRoverEmergency,0x4F, 0,NULL, 8) != pdTRUE) {
						VT_HANDLE_FATAL_ERROR(0);
				}
				break;		
			}		
			case TimerStateMachine: {
			/*
				Problem of the Rover
				1. Almost 2 sec of Sensor Delay
				2. It's not making exact turn
				3. Sensor Range is < 150cm. Over Range will get messy returns.
			
				# Current StateMachine period is 200 ms
			*/
			if ( FrontSensor < 80 && FrontSensor_prev == 110 ) {
					FrontSensor = 110;
			}
			
			if ( stateMachineRunning == 1 ) {
				/* State Machine */
				if ( currentState == start ) {
					sprintf(lcdBuffer, "State: START");
					currentState = idle;
				}
				else if ( currentState == idle ) {
					sprintf(lcdBuffer, "State: IDLE");
					
					if ( counter > 0 ) {
						currentState = check_sensor;
						counter = 0;
					}
					else {
						counter++;
					}
				}
				else if ( currentState == check_sensor) {			
					// Check Front Sensor
					if ( FrontSensor < 60 ) {
							sprintf(lcdBuffer, "State: CHECK_SENSOR-ST");
							roverCommand[0] = i2cCommandST;
							if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
								VT_HANDLE_FATAL_ERROR(0);
							}
							currentState = stop;
						}
						else {
							sprintf(lcdBuffer, "State: CHECK_SENSOR-FW");
							currentState = check_sensor2;
						}
				}
				else if ( currentState == check_sensor2 ) {					// Check 'Left' and 'Rgiht'
					sprintf(lcdBuffer, "State: CHECK_SENSOR2");
					
					if ( LFsensor < 45 ) {									// something detected on the left side
						currentState = stop2;
					}
					else {
						currentState = moveFW;
					}
					
					if ( RBsensor > 40 || RFsensor > 40 ) {
						if ( RBsensor < 50 || RFsensor < 50 ) {
							currentState = moveFW;
						}else {
							currentState = turnRight_s;
							
							if ( RBsensor > 100 && RFsensor > 100 ) {
								currentState = stop3;
							}
						}
					}
					else {
						currentState = turnLeft_s;
					}					
				}
				else if ( currentState == turnLeft_s ) {
					sprintf(lcdBuffer, "State: TLS");
					if ( roverState == 1 ) {
						roverCommand[0] = i2cCommandTLS;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
								VT_HANDLE_FATAL_ERROR(0);
						}	
						currentState = moveForward;
						counter = 0;
					}
				
				}
				else if ( currentState == moveFW ) {
					sprintf(lcdBuffer, "State: moveFW");
					if ( roverState == 1 ) {
						roverCommand[0] = i2cCommandMF;
						roverCommand[1] = 20;
						
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
								VT_HANDLE_FATAL_ERROR(0);
							}	
							
						currentState = idle;
					}
				
				}
				else if ( currentState == parallel ) {
					sprintf(lcdBuffer, "State: PARALLEL");
					if ( roverState == 1 ) {
					
						//int diff = RFsensor - RBsensor;
						//int diff1 = RFsensor - RFsensor_prev;
						int diff2 = RFsensor - RBsensor;
						currentState = idle;
	
						if ( diff2 > 3 && RFsensor < 70 ) {
							roverCommand[0] = i2cCommandTRS;
							currentState = parallel_wait;
						}

						if ( diff2 < -3 && RFsensor < 70 ) {
							roverCommand[0] = i2cCommandTRS;
							currentState = parallel_wait;
						}
						if ( RBsensor < 50 || RFsensor < 50 ) {
							roverCommand[0] = i2cCommandTLS;
							currentState = parallel_wait;
						}
						
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}
						
						sprintf(lcdBuffer, "State: PARALLEL -%d %d", RFsensor, RFsensor_prev);
						
					}
				}
				else if ( currentState == parallel_wait) {
					sprintf(lcdBuffer, "State: PARALLEL_WAIT");
					currentState = idle;
					
					if ( RBsensor < 45 || RFsensor < 45 ) {
						currentState = moveForward;
						
						roverCommand[0] = i2cCommandMF;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}
					}

				}
				else if ( currentState == moveForward) {
					if ( counter > 2 ) {
						currentState = idle;
						counter = 0;
					}
					else {
						counter++;
					}
				}
				else if ( currentState == stop3 ) {
					sprintf(lcdBuffer, "State: STOP3");
					
					if ( counter > 10 ) {
						if (RFsensor > 100 || RBsensor > 100) {
							currentState = turnRight;
							counter = 0;
						}
						else {
							currentState = idle;
							counter = 0;
						}
					}
					else {
						counter ++;
					}
					
				}
				else if ( currentState == turnRight ) {
					sprintf(lcdBuffer, "Turn Right");
					
	
					if ( roverState == 1 ) {
						roverCommand[0] = i2cCommandTR;
						roverCommand[1] = 55;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}	
						currentState = wait_for_turn2;			
					}
					
				}
				else if ( currentState == wait_for_turn2) {
					sprintf(lcdBuffer, "State: wait_for_turn2");
					if ( counter > 5 ) {
						currentState = moveMF_after_right;
						counter = 0;
					}
					else {
						counter++;
					}
				}
				else if ( currentState == moveMF_after_right ) {
						sprintf(lcdBuffer, "State: moveMF_af_right");
						roverCommand[0] = i2cCommandMF;
						roverCommand[1] = 20;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}
						
						if (( RBsensor < 80 ) && ( RFsensor < 80 ) ){
							currentState = idle;
						}
				}
				else if ( currentState == stop2 ) {
					sprintf(lcdBuffer, "State: STOP2");
					
					if ( counter > 2 ) {
						if ( LFsensor < 45 ) {
							currentState = turnRight_s;
							counter = 0;
						}
						else {
							currentState = idle;
							counter = 0;
						}
					}
					else {
						counter++;
					}
				}
				else if ( currentState == turnRight_s ) {
					sprintf(lcdBuffer, "State: TRS");
					if ( roverState == 1 ) {
						roverCommand[0] = i2cCommandTRS;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
								VT_HANDLE_FATAL_ERROR(0);
						}	
						currentState = moveForward;
						counter = 0;
					}
				}
				else if ( currentState == stop ) {
					sprintf(lcdBuffer, "State: STOP");
					if ( counter > 5 ) {
						if ( FrontSensor < 55 ){
							currentState = turnLeft;
						} else {
							roverCommand[0] = i2cCommandMF;
							roverCommand[1] = 20;
							if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
								VT_HANDLE_FATAL_ERROR(0);
							}
							currentState = idle;
						}
							counter = 0;
						
					}
					else {
						counter++;
					}
				
				}
				else if ( currentState == turnLeft ) {
					sprintf(lcdBuffer, "State: TURN LEFT");
					if ( roverState == 1 ) {
						roverCommand[0] = i2cCommandTL;
						roverCommand[1] = 60;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}	
						currentState = wait_for_turn;
					}
				}
				else if ( currentState == wait_for_turn) {
					sprintf(lcdBuffer, "State: WAIT_FOR_TURN");
					if ( counter > 5 ) {
						currentState = adjust;
						counter = 0;
					} 
					else {
						counter ++;
					}
					sprintf(lcdBuffer, "State: WAIT_TURN-%d", counter);
				}
				else if ( currentState == adjust ) {
					sprintf(lcdBuffer, "State: ADJUST");
					int diff = RFsensor - RBsensor;
					
					currentState = idle;
					
					if ( diff > 8 ) {
						sprintf(lcdBuffer, "State: ADJUST_R");
						roverCommand[0] = i2cCommandTRS;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}
						currentState = adjust_wait;
					}
					
					if ( diff < -8 ) {
						sprintf(lcdBuffer, "State: ADJUST_L");
						roverCommand[0] = i2cCommandTLS;
						if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
							VT_HANDLE_FATAL_ERROR(0);
						}	
						currentState = adjust_wait;
					}
				}
				else if ( currentState == adjust_wait ) {
					sprintf(lcdBuffer, "State: ADJUST_WAIT");
					if ( counter > 2 ) {
						currentState = adjust;
						counter = 0;
					}
					else {	
						counter++;
					}
				}

				// Send LCD Message
				if (SendLCDMsgStringDebug(lcdData, strnlen(lcdBuffer, vtLCDMaxLen), lcdBuffer, portMAX_DELAY) != pdTRUE) {
					VT_HANDLE_FATAL_ERROR(0);
				}
				

				FrontSensor_prev = FrontSensor;			
				RFsensor_prev = RFsensor;
				LFsensor_prev = LFsensor;
				
			}
				break;
			}
			case I2CJoystick: {
			
				ret = 0;
				
				if((GPIO_ReadValue(JOYSTICK_UP_GPIO_PORT_NUM) & (1<<JOYSTICK_UP_GPIO_BIT_NUM)) == 0x00)
				{
					roverCommand[0] = i2cCommandMF;
					roverCommand[1] = 20;
					ret++;
				}
				else if((GPIO_ReadValue(JOYSTICK_DOWN_GPIO_PORT_NUM) & (1<<JOYSTICK_DOWN_GPIO_BIT_NUM)) == 0x00)
				{
					roverCommand[0] = i2cCommandMB;
					roverCommand[1] = 20;
					ret++;
				}
				else if((GPIO_ReadValue(JOYSTICK_LEFT_GPIO_PORT_NUM) & (1<<JOYSTICK_LEFT_GPIO_BIT_NUM)) == 0x00)
				{
					roverCommand[0] = i2cCommandTL;		
					roverCommand[1] = 45;
					ret++; 
				}
					else if((GPIO_ReadValue(JOYSTICK_RIGHT_GPIO_PORT_NUM) & (1<<JOYSTICK_RIGHT_GPIO_BIT_NUM)) == 0x00)
				{
					roverCommand[0] = i2cCommandTR;
					roverCommand[1] = 45;
					ret++;
				}
				else if((GPIO_ReadValue(JOYSTICK_PRESS_GPIO_PORT_NUM) & (1<<JOYSTICK_PRESS_GPIO_BIT_NUM)) == 0x00)
				{
					roverCommand[0] = i2cCommandST;
					ret++;
				}
				
				if ( ret != 0) {
					if (vtI2CEnQ(devPtr,I2CRoverCommand,0x4F,sizeof(roverCommand),roverCommand, 0) != pdTRUE) {
						VT_HANDLE_FATAL_ERROR(0);
					}	
				
					sprintf(lcdBuffer, "Button Command.");
					if (SendLCDMsgStringDebug(lcdData, strnlen(lcdBuffer, vtLCDMaxLen), lcdBuffer, portMAX_DELAY) != pdTRUE) {
						VT_HANDLE_FATAL_ERROR(0);
					}
				}
				break;
			}
			case I2CRoverEmergency: {
				if (lcdData != NULL) {
					if(SendLCDMsgReceivedData(lcdData, msgBuffer.length, (char*) msgBuffer.buf, portMAX_DELAY) != pdTRUE) {
						VT_HANDLE_FATAL_ERROR(0);
					}
				}
				
				FrontSensor = msgBuffer.buf[3];
				RBsensor = msgBuffer.buf[0];
				RFsensor = msgBuffer.buf[1];
				LBsensor = msgBuffer.buf[4];
				LFsensor = msgBuffer.buf[2];	
				roverState = msgBuffer.buf[7];
		
				
				
				break;			
			}
			case I2CButtonITR: {
				if ( stateMachineRunning == 1 ) {
					stateMachineRunning = 0;
				}
				else {
					stateMachineRunning = 1;
				}
				break;
			}
			default: {
				VT_HANDLE_FATAL_ERROR(getMsgType(&msgBuffer));	
				break;
			}
		
		}
	}
}	





  