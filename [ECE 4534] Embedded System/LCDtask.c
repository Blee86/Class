#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

/* include files. */
#include "GLCD.h"
#include "vtUtilities.h"
#include "LCDtask.h"
#include "string.h"
#include "supplies_image.h"

// I have set this to a larger stack size because of (a) using printf() and (b) the depth of function calls
//   for some of the LCD operations
// I actually monitor the stack size in the code to check to make sure I'm not too close to overflowing the stack
//   This monitoring takes place if INPSECT_STACK is defined (search this file for INSPECT_STACK to see the code for this) 
#define INSPECT_STACK 1
#define baseStack 3
#if PRINTF_VERSION == 1
#define lcdSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)
#else
#define lcdSTACK_SIZE		(baseStack*configMINIMAL_STACK_SIZE)
#endif

// definitions and data structures that are private to this file
// Length of the queue to this task
#define vtLCDQLen 10 

#define LCDMsgTypeTimer 1
#define LCDMsgTypeString 2
#define LCDMsgTypeStringDebug 3
// a message to be printed
// [1] Info message
#define LCDMsgCommandSent 4
#define LCDMsgReceivedData 5

// Screen Mode
#define LCDMsgTypeSwitchScreen 6

// Scree Mode 
#define ScreenModeMap 0		// Screen 1
#define ScreenModeInfo 1	// Screen 2

// actual data structure that is sent in a message
typedef struct __vtLCDMsg {
	uint8_t msgType;
	uint8_t	length;	 // Length of the message to be printed
	uint8_t buf[vtLCDMaxLen+1]; // On the way in, message to be sent, on the way out, message received (if any)
} vtLCDMsg;
// end of defs

/* definition for the LCD task. */
static portTASK_FUNCTION_PROTO( vLCDUpdateTask, pvParameters );

/*-----------------------------------------------------------*/
// Initialize LCD Task
void StartLCDTask(vtLCDStruct *ptr, unsigned portBASE_TYPE uxPriority)
{ 	
	if (ptr == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	// Create the queue that will be used to talk to this task
	if ((ptr->inQ = xQueueCreate(vtLCDQLen,sizeof(vtLCDMsg))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	/* Start the task */
	portBASE_TYPE retval;
	if ((retval = xTaskCreate( vLCDUpdateTask, ( signed char * ) "LCD", lcdSTACK_SIZE, (void*)ptr, uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}	
}

// LCD Periodic 
portBASE_TYPE SendLCDTimerMsg(vtLCDStruct *lcdData,portTickType ticksElapsed,portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtLCDMsg lcdBuffer;
	lcdBuffer.length = sizeof(ticksElapsed);
	if (lcdBuffer.length > vtLCDMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(lcdBuffer.length);
	}
	memcpy(lcdBuffer.buf,(char *)&ticksElapsed,sizeof(ticksElapsed));
	lcdBuffer.msgType = LCDMsgTypeTimer;
	return(xQueueSend(lcdData->inQ,(void *) (&lcdBuffer),ticksToBlock));
}

// Display String in the bottom
portBASE_TYPE SendLCDMsgString(vtLCDStruct *lcdData,int length,char *pString,portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtLCDMsg lcdBuffer;

	if (length > vtLCDMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(lcdBuffer.length);
	}

	lcdBuffer.length = strnlen(pString, vtLCDMaxLen);
	lcdBuffer.msgType = LCDMsgTypeString;
	strncpy((char*)lcdBuffer.buf,pString, vtLCDMaxLen);
	return(xQueueSend(lcdData->inQ,(void *) (&lcdBuffer),ticksToBlock));
}

// Display String in the bottom
portBASE_TYPE SendLCDMsgStringDebug(vtLCDStruct *lcdData,int length,char *pString,portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtLCDMsg lcdBuffer;

	if (length > vtLCDMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(lcdBuffer.length);
	}

	lcdBuffer.length = strnlen(pString, vtLCDMaxLen);
	lcdBuffer.msgType = LCDMsgTypeStringDebug;
	strncpy((char*)lcdBuffer.buf,pString, vtLCDMaxLen);
	return(xQueueSend(lcdData->inQ,(void *) (&lcdBuffer),ticksToBlock));
}
// Sent Command to LCD
portBASE_TYPE SendLCDMsgCommandSent(vtLCDStruct *lcdData,int length,char *msgbuf, portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtLCDMsg lcdBuffer;

	if (length > vtLCDMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(lcdBuffer.length);
	}
	lcdBuffer.length = length;
	lcdBuffer.msgType = LCDMsgCommandSent;

	memcpy(&lcdBuffer.buf, msgbuf, length);
	return(xQueueSend(lcdData->inQ,(void *) (&lcdBuffer),ticksToBlock));
}


portBASE_TYPE SendLCDMsgReceivedData(vtLCDStruct *lcdData,int length,char *msgbuf, portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	vtLCDMsg lcdBuffer;

	if (length > vtLCDMaxLen) {
		// no room for this message
		VT_HANDLE_FATAL_ERROR(lcdBuffer.length);
	}
	lcdBuffer.length = length;
	lcdBuffer.msgType = LCDMsgReceivedData;
	
	memcpy(&lcdBuffer.buf, msgbuf, length);
	return(xQueueSend(lcdData->inQ,(void *) (&lcdBuffer),ticksToBlock));
}

/* --------------------------------------------------------------
Send a LCD Screen Switch signal to LCD Task
--------------------------------------------------------------- */
portBASE_TYPE SendLCDSwitchMsg(vtLCDStruct *lcdData,portTickType ticksToBlock)
{
	if (lcdData == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	vtLCDMsg lcdBuffer;
	
	lcdBuffer.msgType = LCDMsgTypeSwitchScreen;
	return(xQueueSend(lcdData->inQ, (void *) (&lcdBuffer), ticksToBlock));
}

// Private routines used to unpack the message buffers
//   I do not want to access the message buffer data structures outside of these routines
portTickType unpackTimerMsg(vtLCDMsg *lcdBuffer)
{
	portTickType *ptr = (portTickType *) lcdBuffer->buf;
	return(*ptr);
}

int getMsgType(vtLCDMsg *lcdBuffer)
{
	return(lcdBuffer->msgType);
} 

int getMsgLength(vtLCDMsg *lcdBuffer)
{
	return(lcdBuffer->msgType);
}

void copyMsgString(char *target,vtLCDMsg *lcdBuffer,int targetMaxLen)
{
	strncpy(target,(char *)(lcdBuffer->buf),targetMaxLen);
}

// End of private routines for message buffers

// If LCD_EXAMPLE_OP=0, then accept messages that may be timer or print requests and respond accordingly
// If LCD_EXAMPLE_OP=1, then do a rotating ARM bitmap display
#define LCD_EXAMPLE_OP 0

static unsigned short hsl2rgb(float H,float S,float L);

#if LCD_EXAMPLE_OP==0
// Buffer in which to store the memory read from the LCD
	#define MAX_RADIUS 15
	#define BUF_LEN (((MAX_RADIUS*2)+1)*((MAX_RADIUS*2)+1))
	static unsigned short int buffer[BUF_LEN];
#endif

// This is the actual task that is run
static portTASK_FUNCTION( vLCDUpdateTask, pvParameters )
{
	// I don't know exactly what this is doing..
	unsigned short screenColor = 0;
	unsigned short tscr;

	
	vtLCDMsg msgBuffer;
	vtLCDStruct *lcdPtr = (vtLCDStruct *) pvParameters;

	#ifdef INSPECT_STACK 
	unsigned portBASE_TYPE InitialStackLeft = uxTaskGetStackHighWaterMark(NULL);
	unsigned portBASE_TYPE CurrentStackLeft;
	float remainingStack = InitialStackLeft;
	remainingStack /= lcdSTACK_SIZE;
	if (remainingStack < 0.10) {
		// If the stack is really low, stop everything because we don't want it to run out
		// The 0.10 is just leaving a cushion, in theory, you could use exactly all of it
		VT_HANDLE_FATAL_ERROR(0);
	}
	#endif

	/* Initialize the LCD and set the initial colors */

	GLCD_Init();
	// Screen Mode
	uint8_t screen_mode = ScreenModeInfo;
	tscr = White; // may be reset in the LCDMsgTypeTimer code below
	screenColor = Black; // may be reset in the LCDMsgTypeTimer code below
	GLCD_SetTextColor(tscr);
	GLCD_SetBackColor(screenColor);
	GLCD_Clear(screenColor);
	GLCD_DisplayString(0, 22, 0, (unsigned char *) "[Debug]");
	// String Buffer
	char lineBuffer[lcdCHAR_IN_LINE_SMALL + 1];
	char cmdBuffer1[lcdCHAR_IN_LINE_SMALL + 1];
	char cmdBuffer2[lcdCHAR_IN_LINE_SMALL + 1];
	uint8_t currLine = 1;
	
	for(;;)
	{	
		#ifdef INSPECT_STACK   
		CurrentStackLeft = uxTaskGetStackHighWaterMark(NULL);
		float remainingStack = CurrentStackLeft;
		remainingStack /= lcdSTACK_SIZE;
		
		if (remainingStack < 0.10) {
			// If the stack is really low, stop everything because we don't want it to run out
			VT_HANDLE_FATAL_ERROR(0);
		}
		#endif

		// Wait for a message
		if (xQueueReceive(lcdPtr->inQ,(void *) &msgBuffer,portMAX_DELAY) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}
		
		//Log that we are processing a message -- more explanation of logging is given later on
		vtITMu8(vtITMPortLCDMsg,getMsgType(&msgBuffer));
		vtITMu8(vtITMPortLCDMsg,getMsgLength(&msgBuffer));


		switch(getMsgType(&msgBuffer)) {
		case LCDMsgTypeTimer: {
			break;
		}
		case LCDMsgTypeString: {
			copyMsgString(lineBuffer, &msgBuffer, lcdCHAR_IN_LINE_SMALL);
			GLCD_ClearLn( 29, 0);
			GLCD_DisplayString(29,0,0,(unsigned char *)lineBuffer);
			break;
		}
		case LCDMsgTypeStringDebug: {
			copyMsgString(lineBuffer, &msgBuffer, lcdCHAR_IN_LINE_SMALL);
			GLCD_DisplayString(currLine,20,0,(unsigned char *)lineBuffer);
			
			if (++currLine > 26) {
			GLCD_ClearWindow(100, 0, 160, 239, Black);
			GLCD_DisplayString(0, 22, 0, (unsigned char *) "[Debug]");
			currLine = 1;
			}
			break;
		}
		case LCDMsgCommandSent: {
			if ( screen_mode == ScreenModeInfo) {
				
				GLCD_DisplayString(3, 0, 0, (unsigned char*) "[Command1]");
				
				if (msgBuffer.buf[0] == 0xAA ) {
					sprintf(cmdBuffer1, "Update Data ");
				} else if ( msgBuffer.buf[0] == 0xA6) {
					sprintf(cmdBuffer1, "Move Forward");
				} else if ( msgBuffer.buf[0] == 0xA7) {
					sprintf(cmdBuffer1, "Turn Right ");
				} else if ( msgBuffer.buf[0] == 0xA8) {
					sprintf(cmdBuffer1, "Stop       ");
				} else if ( msgBuffer.buf[0] == 0xA9) {
					sprintf(cmdBuffer1, "Turn Left  ");
				}
				
				sprintf(cmdBuffer1, "0x%02x", msgBuffer.buf[0]);
				GLCD_DisplayString(4, 2, 0, (unsigned char*) cmdBuffer1);
				
				
				GLCD_DisplayString(5, 0, 0, (unsigned char*) "[Command2]");
				
				sprintf(cmdBuffer2, "0x%02x", msgBuffer.buf[1]);
				GLCD_DisplayString(6, 2, 0, (unsigned char*) cmdBuffer2);
			}
			break;
		}
		case LCDMsgReceivedData: {
			if ( screen_mode == ScreenModeInfo) {
				char lineBuffer2[lcdCHAR_IN_LINE_SMALL + 1];	// SENSORS1
				char lineBuffer3[lcdCHAR_IN_LINE_SMALL + 1];	// SENSORS2
				char lineBuffer4[lcdCHAR_IN_LINE_SMALL + 1];	// COUNT

				sprintf(lineBuffer,  "     F    R");
				sprintf(lineBuffer2, "L: %03d %03d", msgBuffer.buf[2], msgBuffer.buf[4]);
				sprintf(lineBuffer3, "R: %03d %03d",  msgBuffer.buf[1], msgBuffer.buf[0]);	
				sprintf(lineBuffer4, "F: %03d", msgBuffer.buf[3]); 		

				GLCD_DisplayString(7, 0,  0, (unsigned char*) lineBuffer);
				GLCD_DisplayString(8, 0,  0, (unsigned char*) lineBuffer2);
				GLCD_DisplayString(9, 0,  0, (unsigned char*) lineBuffer3);
				GLCD_DisplayString(10, 0,  0, (unsigned char*) lineBuffer4);
			}
			break;
		}
		case LCDMsgTypeSwitchScreen: {
			vtLEDToggle(8);
			/*
			if ( screen_mode == ScreenModeMap ) {
				GLCD_Clear(screenColor);
				screen_mode = ScreenModeInfo; 
			}
			else {
				GLCD_SetTextColor(tscr);
				GLCD_SetBackColor(screenColor);
				GLCD_Clear(screenColor);
				GLCD_DrawXYGraph();
				screen_mode = ScreenModeMap;
			}
			
			break;
			*/
			break;
		}
		
		default: {
		 	VT_HANDLE_FATAL_ERROR(getMsgType(&msgBuffer));
			break;
		}
	
	}
}
}


// Convert from HSL colormap to RGB values in this weird colormap
// H: 0 to 360
// S: 0 to 1
// L: 0 to 1
// The LCD has a funky bitmap.  Each pixel is 16 bits (a "short unsigned int")
//   Red is the most significant 5 bits
//   Blue is the least significant 5 bits
//   Green is the middle 6 bits
static unsigned short hsl2rgb(float H,float S,float L)
{
	float C = (1.0 - fabs(2.0*L-1.0))*S;
	float Hprime = H / 60;
	unsigned short t = Hprime / 2.0;
	t *= 2;
	float X = C * (1-abs((Hprime - t) - 1));
	unsigned short truncHprime = Hprime;
	float R1, G1, B1;

	switch(truncHprime) {
		case 0: {
			R1 = C; G1 = X; B1 = 0;
			break;
		}
		case 1: {
			R1 = X; G1 = C; B1 = 0;
			break;
		}
		case 2: {
			R1 = 0; G1 = C; B1 = X;
			break;
		}
		case 3: {
			R1 = 0; G1 = X; B1 = C;
			break;
		}
		case 4: {
			R1 = X; G1 = 0; B1 = C;
			break;
		}
		case 5: {
			R1 = C; G1 = 0; B1 = X;
			break;
		}
		default: {
			// make the compiler stop generating warnings
			R1 = 0; G1 = 0; B1 = 0;
			VT_HANDLE_FATAL_ERROR(Hprime);
			break;
		}
	}
	float m = L - 0.5*C;
	R1 += m; G1 += m; B1 += m;
	unsigned short red = R1*32; if (red > 31) red = 31;
	unsigned short green = G1*64; if (green > 63) green = 63;
	unsigned short blue = B1*32; if (blue > 31) blue = 31;
	unsigned short color = (red << 11) | (green << 5) | blue;
	return(color); 
}					

