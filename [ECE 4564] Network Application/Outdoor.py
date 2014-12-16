"""
    Team Raven - Personal Weather Station 'Outdoor'
    - Collecting Weather Information from Free Weather API, World Weather Online
    - Collecting data from client weather stations

    World Weather Online
    URL: https://developer.worldweatheronline.com

    With Free membership
    Queries per day :    5
    Queries per day :   12,000

    FYI. The weather server is based on London time. So 'Observation_time' Value is
    not wrong number.

    Author  : Yosub Lee
    Date    : 11/27/2014

"""
import json
import urllib
import urllib2
import datetime
import argparse
import signal
import sys
import time
from LED import LEDcolor
from pprint import pprint

# Global Variables #######################
serviceOn = True
verbose = False
"""
It is the same way we did for Assignment 2. Using the signal,
it will toggle the global boolean value, 'servinceOn.'
"""
def stop_service(signal, frame):
    """
    a signal handler, that will stop updateJson()

    :param signal: (int) A number if a intercepted signal caused this handler
                   to be run, otherwise None
    :param frame: A Stack Frame object, if an intercepted signal caused this
                  handler to be run
    :return: None
    """
    global serviceOn
    serviceOn = False

def updateJson(API_Key, address, zipcode, numOfDays):
    """
    Gets current Weather data and forecast for next four days
    the collected data is stored in 'outdoor_raw.json'

    today's weather data is stored in 'outdoor_current.json'
    forecast data is stored in 'outdoor_forecast.json'

    :param API_Key: (str) Key code provided from the website 'worldweatheronline.'
    :param address: (str) Host address of service
    :param zipcode: (str) Target Zipcode
    :param numOfDays: (int) number of days for forecast data. Should be between 1 and 5

    :return: None
    """

    try:
        global verbose
        query_args = {'q': zipcode, 'num_of_days': numOfDays, 'format': 'json', 'key': API_Key}
        encoded_args = urllib.urlencode(query_args)
        url = address + encoded_args

        # GET response
        if verbose is True:
            print "GET = ", url
        response = json.loads(urllib2.urlopen(url).read())

        # Save As Json File
        output = open('outdoor_raw.json', 'w')
        json.dump(response, output)

        # read Json file
        current = response["data"]["current_condition"][0]

        ### Outdoor - Current ###
        # Update observation time to current time
        currentHour = datetime.datetime.now().hour
        currentMin = datetime.datetime.now().minute
        currentTime = "%s:%s" %(currentHour, currentMin)

        current['observation_time'] = currentTime
        current.update({"zipcode":zipcode})
        current_weather = open('outdoor_current.json', 'w')
        json.dump(current, current_weather)
        current_weather.close()

        if verbose is True:
            pprint(current)
            print '"outdoor_current.json" is updated.'

        # Change LED Color
        curr_temp = float(current['temp_F'])

        #
        if curr_temp < 34:
            LEDcolor("000")
        elif curr_temp > 33:
            if curr_temp > 75:
                LEDcolor("011")
            else:
                LEDcolor("110")

        ### Outdoor (5 days) - Hourly ###
        forecast_data = open('outdoor_forecast.json', 'w')
        future = response["data"]["weather"]

        # Extract & Organize Data from response (5 days)
        forecast = {}

        for x in xrange(0, 5):
            forecast[x] = {"date": future[x]["date"],
                           "max": future[x]["maxtempF"],
                           "min": future[x]["mintempF"],
                           "zipcode": zipcode}

        json.dump(forecast, forecast_data)
        forecast_data.close()
        if verbose is True:
            print '"outdoor_forecast.json" is updated.'

    except urllib2.HTTPError, e:
        print 'HTTP Error = ', e
    except urllib2.URLError, e:
        print 'URL Error = ' , e
    except urllib.HTTPException, e:
        print 'HTTP Exception = ', e
    except Exception:
        import traceback
        print "Exception = ", traceback.format_exc()

# Main Program ##
try:
    API_Key = "244d46fa959e38ad6234ce9bbd755"
    address = "http://api.worldweatheronline.com/free/v2/weather.ashx?"
    zipcode = None
    repeat_time = None
    numOfDays = 5

    signal_num = signal.SIGINT

    try:
        signal.signal(signal_num, stop_service)
        signal_num = signal.SIGTERM
        signal.signal(signal_num, stop_service)
    except ValueError, ve:
        print "Warning: Greceful shutdown may not be possible: Unsupported " \
              "Signal: " + signal_num

    # Argument Parser
    parser = argparse.ArgumentParser(description= "usage: outdoor.py -z zipcode [-k API_Key] [-r repeat_time in min] [-vv]",
                               version = "outdoor v0.1")

    parser.add_argument("-z",
                  action="store",
                  dest= "zipcode",
                  help="Zipcode of the place to collect the weather data")

    parser.add_argument("-k",
                  dest="Key",
                  default="244d46fa959e38ad6234ce9bbd755",
                  help = "API Key from 'worldweatheronline.com'")

    parser.add_argument("-r",
                    dest="Repeat",
                    default="10",
                    help = "Repeat Time in min. Decimal Point is allowed")

    parser.add_argument("-vv",
                        action="store_true",
                        dest="Verbose",
                        help="Verbose Mode"
    )

    arguments = parser.parse_args()
    verbose = arguments.Verbose
    API_Key = arguments.Key
    zipcode = arguments.zipcode
    try:
        repeat_time = float(arguments.Repeat)*60
    except ValueError:
        print 'Repeat_time must be float number in min.'

    if zipcode is None:
        raise Exception("Zipcode must be specified!")


    print "Zipcode: ", zipcode
    print "API_Key: ", API_Key
    print "Repeat Time: ", repeat_time, 'sec'
    print "Starting the application . . ."
    print "-------------------------------"
    # Starting Program!
    while(serviceOn):
        updateJson(API_Key, address, zipcode, numOfDays)
        if verbose is True:
            print " "
        time.sleep(repeat_time)
        
    print "Application Stopped by User"

except Exception, e:
    print e
    sys.exit()


