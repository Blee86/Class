"""
    Author: Pedro Sorto
"""

import os

##os.system("sudo pigpiod")
import json
import io
import time
import pigpio
import pika

# RabbitMQ 
host = '172.31.169.144'
port = 5672
topic = 'Indoor.data'
routing_key = "Indoor"

print "running pigpiod"
time.sleep(3)
newline = "\\n"


print "pigpiod ran successfully"
time.sleep(2)
pi = pigpio.pi()
time.sleep(2)
import DHT22
time.sleep(2)
s = DHT22.sensor(pi, 4)
time.sleep(2)

count = 0;
while(True):
	os.system("echo 0 > /sys/class/gpio/gpio17/value")	
	os.system("echo 0 > /sys/class/gpio/gpio22/value")	
	s.trigger()
	time.sleep(2)
	number = str(count)
	print "Reading number: " + number
	print "Humidity: "
	hum = str('{:3.2f}'.format(s.humidity() / 1.))
	print ("                               " + hum)
	print "Temperature: "
	##print('{:3.2f}'.format(s.temperature() / 1.))
	tem = str('{:3.2f}'.format(s.temperature() / 1.))
	print ("    Degree Celsius:            " + tem)
	tem_c = '{:3.2f}'.format(s.temperature() / 1.)
	TemC = float(tem_c)
	tem_f = ((TemC*9)/5) + 32
	temFString = str(tem_f)
	print ("    Degrees Fahrenheit:        " + temFString)
	entry = {"H": hum,"T": tem,"Name": "dev1"}
	print " "
	if(tem_f > 80):
		os.system("echo 1 > /sys/class/gpio/gpio17/value")
	else:
		os.system("echo 1 > /sys/class/gpio/gpio22/value")		
	
	# send it to RabbitMQ Server
	json_format = json.dumps(entry)
	message_broker = pika.BlockingConnection(pika.ConnectionParameters(host, port))
	channel = message_broker.channel()
	channel.exchange_declare(exchange=topic, type='topic')
	
	channel.basic_publish(exchange = topic, routing_key = routing_key, body = json_format)

	print "Sent: ", entry	
	time.sleep(10)                            ## change this delay to determine the interval
        

    	"""
   	with open('data.json','r') as feeds:
		data = json.load(feeds)
		
   	data.append(entry)
  	

	with open('data.json', 'w') as feeds:
        	#json.dump(data, feeds, indent = 1)
	"""


	count = count + 1
	
print "Stopped reading"
os.system("echo 0 > /sys/class/gpio/gpio17/value")	
