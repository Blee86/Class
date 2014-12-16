/*
	[ Team 8, The (below) average guys ]
	External Interrupt Button

	source is from
		http://www.emblocks.org/web/downloads-main/examples/file/3-mcb1700-eti-example?tmpl=component

	Yosub Lee
	Date: 10/8/2014
*/
#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "vtUtilities.h"
#include "lcdTask.h"
#include "LPC17xx.h"

#include "i2cSensor.h"
void EINT3_IRQHandler() __attribute__ ((externally_visible));

//vtLCDStruct *lcd;
vtSensorStruct* sensor;
// Initialize Button ( INT0 Port 2.10 )
// Pressing INT0 will change a CurrentState of LCD Task

//void Button_Init(vtLCDStruct *lcdStruct) {
void Button_Init(vtSensorStruct *sensorStruct){
  //lcd = lcdStruct;
  sensor = sensorStruct;
  LPC_GPIO2->FIODIR      &= ~(1 << 10);    /* PORT2.10 defined as input       */
  LPC_GPIOINT->IO2IntEnF |=  (1 << 10);    /* enable falling edge irq         */

  NVIC_EnableIRQ(EINT3_IRQn);              /* enable irq in nvic              */
}
void EINT3_IRQHandler()
{
  
  LPC_GPIOINT->IO2IntClr |= (1 << 10);     /* clear pending interrupt         */
	
	SendButtonSignal(sensor, portMAX_DELAY);
  // Put a signal into LCD Queue
  // SendLCDSwitchMsg(lcd, portMAX_DELAY);
  // For now, it's just for Debugging
 
  
}
