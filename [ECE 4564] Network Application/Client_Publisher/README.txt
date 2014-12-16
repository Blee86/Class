$ cd ~
$ wget abyz.co.uk/rpi/pigpio/pigpio.zip
$ unzip pigpio.zip	
$ cd PIGPIO
$ make
$ make install

download DHT22 example code here: http://abyz.co.uk/rpi/pigpio/code/DHT22_py.zip

##must run the following lines in the terminal for the LEDs to work
echo 17 > /sys/class/gpio/export 
echo 22 > /sys/class/gpio/export 
echo out > /sys/class/gpio/gpio17/direction
echo out > /sys/class/gpio/gpio22/direction

$ sudo pigpiod
$ cd ~

download my code to root folder
$ sudo python piSensor.py