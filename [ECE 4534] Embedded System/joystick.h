#ifndef __JoyStick_H__
#define __JoyStick_H__

#include "vtUtilities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lpc17xx_gpio.h"

#define JOYSTICK_UP_GPIO_PORT_NUM				1
#define JOYSTICK_UP_GPIO_BIT_NUM				24
#define JOYSTICK_DOWN_GPIO_PORT_NUM				1
#define JOYSTICK_DOWN_GPIO_BIT_NUM				26
#define JOYSTICK_LEFT_GPIO_PORT_NUM				1
#define JOYSTICK_LEFT_GPIO_BIT_NUM				23
#define JOYSTICK_RIGHT_GPIO_PORT_NUM			1
#define JOYSTICK_RIGHT_GPIO_BIT_NUM				25
#define JOYSTICK_PRESS_GPIO_PORT_NUM			1
#define JOYSTICK_PRESS_GPIO_BIT_NUM				20

#endif
