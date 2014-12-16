
#include "joystick.h"

void Joystick_Init(void)
{
	GPIO_SetDir(JOYSTICK_UP_GPIO_PORT_NUM,(1<<JOYSTICK_UP_GPIO_BIT_NUM),0); 		// input
	GPIO_SetDir(JOYSTICK_DOWN_GPIO_PORT_NUM,(1<<JOYSTICK_DOWN_GPIO_BIT_NUM),0); 	// input
	GPIO_SetDir(JOYSTICK_LEFT_GPIO_PORT_NUM,(1<<JOYSTICK_LEFT_GPIO_BIT_NUM),0); 	// input
	GPIO_SetDir(JOYSTICK_RIGHT_GPIO_PORT_NUM,(1<<JOYSTICK_RIGHT_GPIO_BIT_NUM),0); 	// input
	GPIO_SetDir(JOYSTICK_PRESS_GPIO_PORT_NUM,(1<<JOYSTICK_PRESS_GPIO_BIT_NUM),0); 	// input
}

	/*		  
uint8_t Joystick_GetStatus(void)
{
	uint8_t ret = NO_BUTTON_PRESSED;

	if((GPIO_ReadValue(JOYSTICK_UP_GPIO_PORT_NUM) & (1<<JOYSTICK_UP_GPIO_BIT_NUM)) == 0x00)
	{
		ret |= JOY_UP;
	}
	else if((GPIO_ReadValue(JOYSTICK_DOWN_GPIO_PORT_NUM) & (1<<JOYSTICK_DOWN_GPIO_BIT_NUM)) == 0x00)
	{
		ret |= JOY_DOWN;
	}
	else if((GPIO_ReadValue(JOYSTICK_LEFT_GPIO_PORT_NUM) & (1<<JOYSTICK_LEFT_GPIO_BIT_NUM)) == 0x00)
	{
		ret |= JOY_LEFT;
	}
	else if((GPIO_ReadValue(JOYSTICK_RIGHT_GPIO_PORT_NUM) & (1<<JOYSTICK_RIGHT_GPIO_BIT_NUM)) == 0x00)
	{
		ret |= JOY_RIGHT;
	}
	else if((GPIO_ReadValue(JOYSTICK_PRESS_GPIO_PORT_NUM) & (1<<JOYSTICK_PRESS_GPIO_BIT_NUM)) == 0x00)
	{
		ret |= JOY_PRESS;
	}

	return ret;
}
*/