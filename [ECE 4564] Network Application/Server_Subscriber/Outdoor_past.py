__author__ = 'yosublee86'

"""
    This program is designed to get the max temperature & the min temperature of
    past 5 years from today. Collected data is sotred in JSON format.
    whenever this program runs it will generate a file named "outdoor_past.json"

    Author  : Yosub Lee
    Date    : 12/12/2014
"""

import urllib
import urllib2
import json
import datetime
import signal
import sys
import time
import argparse
from pprint import pprint
from time import sleep

# Global Variables
serviceOn = True
verbose = False
repeat_time = 600   # every 10 min


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

def updateJson(zipcode):
    """
    Gets Past weather data of the same day of past five years
    The result is stored in "outdoor_past.json" file
    :param zipcode: (Str) Zipcode
    :return: None
    """
    DAILY_SUM = "GHCND" # Daily_data
    LIMIT = 100
    CDOaddr = "http://www.ncdc.noaa.gov/cdo-web/api/v2/data?"
    TOKEN = "dfYSHDPQRpsMcDEZMWdLDOmpJoYWCaNi"
    collection = dict()

    # Date of Today
    thisYear = datetime.datetime.now().date().year
    thisMonth = datetime.datetime.now().date().month
    thisDay = datetime.datetime.now().date().day

    # Last Year
    lastYear = thisYear - 1
    lastMonth = thisMonth
    lsatDay = None
    # Consider Leap year
    if thisMonth is 2 and thisDay is 29:
        lastDay = thisDay - 1
    else:
        lastDay = thisDay

    # Request queries (past 5 years) & Collected data into Json file
    for i in xrange(0,5):
        date = "{0}-{1}-{2}".format((lastYear-i), lastMonth, lastDay)
        query = urllib.urlencode({"datasetid": DAILY_SUM,
                                  "locationid": "ZIP:"+zipcode,
                                  "startdate":str(date),
                                  "enddate":str(date),
                                  "limit":LIMIT,
                                  "offset":0})

        request = urllib2.Request(CDOaddr + query)
        request.add_header("token", TOKEN)

        client = urllib2.urlopen(request)
        response = json.loads(client.read())

        # Parsing the result
        try:
            if response is not None:
                results = response['results']
                length = len(results)
                TMAX = None
                TMIN = None

                for j in xrange(0, length):
                    if results[j]['datatype'] == 'TMAX':
                        TMAX = results[j]['value']
                    elif results[j]['datatype'] == 'TMIN':
                        TMIN = results[j]['value']

                    if j == (length - 1):
                        collection[i] = {"date": date,
                                          "TMAX": TMAX,
                                          "TMIN": TMIN,
                                          "zipcode": zipcode}
            else:
                print "response None"
                pprint(results)

            sleep(0.2)
        except urllib2.HTTPError, e:
            print 'HTTP Error = ', e
        except urllib2.URLError, e:
            print 'URL Error = ' , e
        except urllib.HTTPException, e:
            print 'HTTP Exception = ', e
        except Exception:
            import traceback
            print "Exception = ", traceback.format_exc()

        # Update outdoor_past.json file
        if collection is not None:
            output = open('outdoor_past.json', 'w')
            json.dump(collection, output)
            output.close()

    if verbose is True:
        print "outdoor_past.json Updated"

"""
Main Program
"""
try:

    signal_num = signal.SIGINT

    try:
        signal.signal(signal_num, stop_service)
        signal_num = signal.SIGTERM
        signal.signal(signal_num, stop_service)
    except ValueError, ve:
        print "Warning: Greceful shutdown may not be possible: Unsupported " \
              "Signal: " + signal_num

    # Argument Parser
    parser = argparse.ArgumentParser(description= "usage: NOAA.py -z zipcode [-vv]",
                               version = "NOAA v0.1")

    parser.add_argument("-z",
                  action="store",
                  dest= "zipcode",
                  help="Zipcode of the place to collect the weather data")

    parser.add_argument("-vv",
                        action="store_true",
                        dest="Verbose",
                        help="Verbose Mode")

    arguments = parser.parse_args()
    zipcode = arguments.zipcode
    verbose = arguments.Verbose

    if zipcode is None:
        raise Exception("usage: NOAA.py -z zipcode [-vv]")

    if verbose is True:
        print "Past Data Collector Started . . ."

    # Starting Program!
    while(serviceOn):

        updateJson(zipcode)

        if verbose is True:
            print " "
        time.sleep(repeat_time)

    print "Application Stopped by User"


except Exception, e:
    print e
    sys.exit()

