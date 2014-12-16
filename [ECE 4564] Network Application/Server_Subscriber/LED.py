"""
Author	= Yosub Lee
Date	= 12/14/2014

RGB LED Control

This code is based on the YouTube Video:
https://www.youtube.com/watch?v=b4_R1eX9K6s

"""
import RPi.GPIO as GPIO

PINnumber1 = 16
PINnumber2 = 18
PINnumber3 = 22

GPIO.setmode(GPIO.BOARD)

GPIO.setup(PINnumber1, GPIO.OUT)
GPIO.output(PINnumber1, 1)
GPIO.setup(PINnumber2, GPIO.OUT)
GPIO.output(PINnumber2, 1)
GPIO.setup(PINnumber3, GPIO.OUT)
GPIO.output(PINnumber3, 1)

"""
Method
"""

def LEDcolor(request):
	"""
	0 is On
	1 is Off
	Following is the combination of color code.

	011 - Red
	101 - Green
	110 - Blue
	010 - Purple
	000 - White
	:param request: (str) 3-digit number. 0 means on. 1 means off. 1st - Red, 2nd - Green, 3rd - Blue
	:return: None
	"""
	if (len(request) == 3):
		GPIO.output(PINnumber1, int(request[0]))
		GPIO.output(PINnumber2, int(request[1]))
		GPIO.output(PINnumber3, int(request[2]))

def LEDclose():
	"""
	Call this method before you quit the program
	:return: None
	"""
	GPIO.cleanup()


