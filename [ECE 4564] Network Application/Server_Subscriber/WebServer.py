####################################################################################
#   ECE 4564 Final Project - Web Server Part
#   Author: Han Seung Lee
#   Description:
#       When the server rasp get and store the weather data,
#       Web Server will arrange that data to the web for monitoring
####################################################################################

__author__ = 'Han Seung Lee'

from twisted.web import server, resource
from twisted.internet import reactor
from pprint import pprint
import json
import datetime
import csv

class Simple(resource.Resource):
    isLeaf = True

    def render_GET(self, request):
        print "HTTP GET.."
        # Make Weather Data to HTML Format
        # Forecast Weather
        current_data = getWeatherData(path_current)
        forecast_data = getWeatherData(path_forecast)
        past_data = getWeatherData(path_past)
        indoor_dev1_data = getWeatherData(path_indoor_dev1)
        indoor_dev2_data = getWeatherData(path_indoor_dev2)

        # Get city by zip code
        #convertToJSON("zip_code.json")
        zip_data = getZipData2(current_data['zipcode'])

        # Generate HTML
        forecast_html = htmlForecast(forecast_data)
        current_html = htmlCurrent(current_data)
        past_html = htmlPast(past_data)
        indoor_html1 = htmlindoor(indoor_dev1_data)
        indoor_html2 = htmlindoor(indoor_dev2_data)

        headhtml = "<!DOCTYPE html>\n" \
                   "<html>\n" \
                   "<head>\n" \
                   "    <title> Personal Weather Station </title>\n" \
                   '    <link rel="stylesheet" type="text/css" href="style.css">\n' \
                   "</head>\n"

        bodyhtml = '<body align = "center">\n' \
                   '<div align="left" style="border: 1px solid black; padding-bottom: 92px; height:100%; width:25%; float:left; margin:5px;">\n'\
                   '<h2 align ="center"> Current Weather </h2>\n' \
                   + str(current_html) + \
                   '</div>\n'\
                   '<div align="left" style="border: 1px solid black; padding-bottom: 30px; width:50%; float:left; margin:5px;">\n'\
                   '<h2 align ="center"> Forecast Weather</h2>\n' \
                   + str(forecast_html) + \
                   '</div>\n'\
                   '<div align="left" style="border: 1px solid black; padding-bottom: 30px; width:50%; float:left; margin:5px;">\n'\
                   '<h2 align ="center"> Historical Weather of Today </h2>\n' \
                   + str(past_html) + \
                   '</div>\n'\
                   '<div align="left" style="border: 1px solid red; padding-bottom: 30px; width:25%; margin:5px; clear:left; float:left;">\n'\
                   '<h2 align ="center"> Indoor Status </h2>\n '\
                   '<table align = "center" border="1">\n' \
                   '    <tr>\n' \
                   '        <th></th>\n' \
                   '        <th>Temperature</th>\n' \
                   '        <th>Humidity</th>\n' \
                   '    </tr>\n' \
                   + str(indoor_html1) + '\n' \
                   '    <tr>\n' \
                   '        <td>dev2</td>\n' \
                   '        <td> Not Connected </td>\n' \
                   '        <td> Not Connected </td>\n' \
                   '    </tr>\n'\
                   '</table>\n' \
                   '</div>\n'\
                   '<div align="center" style="border: 1px solid black; padding-top: 7px; ' \
                   '                    padding-bottom: 7px; width:50%; float:left; margin:5px;">\n'\
                   '<h2> Location: ' + str(zip_data['city']) + ', ' + str(zip_data['state']) + '</h2> \n'\
                   '<h2> Latitude: ' + str(zip_data['latitude']) + '</h2> \n'\
                   '<h2> Longitude: ' + str(zip_data['longitude']) + '</h2> \n'\
                   '</div>\n'\
                   '<div align="center" style="float:left; width:100%;"><h4> Data is provided by NOAA & World Weather Online </h4></div>\n'\
                   "</body>\n" \
                   "</html>"

        html = headhtml + bodyhtml
        return html


# Get Data from JSON file (JSON format)
def getWeatherData(path):
    # Open a file
    file=open(path, "r")
    data = json.load(file)
    file.close()
    return data


# Get City information from CSV file
def getZipData2(zipcode):
    # Open a file
    file=open("zip_code.csv", "r")

    field = ("zip","city","state","latitude","longitude")
    reader = csv.DictReader(file, field)

    for row in reader:
        if (row['zip'] == zipcode):
            return row

    return -1


# Conver CSV file to JSON file
def convertToJSON(path):
    #Used to convert csv file to JSON
    csvfile = open('zip_code.csv', 'r')
    jsonfile = open('zip_code.json', 'w')

    field = ("zip","city","state","latitude","longitude")
    reader = csv.DictReader(csvfile, field)
    for row in reader:
        json.dump(row, jsonfile)
        jsonfile.write('\n')

    csvfile.close()
    jsonfile.close()


# Make HTML Format for Forecast Weather
def htmlForecast(data):
    date0 = datetime.datetime.strptime(data['0']['date'], "%Y-%m-%d")
    date1 = datetime.datetime.strptime(data['1']['date'], "%Y-%m-%d")
    date2 = datetime.datetime.strptime(data['2']['date'], "%Y-%m-%d")
    date3 = datetime.datetime.strptime(data['3']['date'], "%Y-%m-%d")
    date4 = datetime.datetime.strptime(data['4']['date'], "%Y-%m-%d")
    weekday0 = datetime.date.weekday(date0)

    # Max Temperature
    maxtemp0 = data['0']['max']
    maxtemp1 = data['1']['max']
    maxtemp2 = data['2']['max']
    maxtemp3 = data['3']['max']
    maxtemp4 = data['4']['max']

    # Min Temperature
    mintemp0 = data['0']['min']
    mintemp1 = data['1']['min']
    mintemp2 = data['2']['min']
    mintemp3 = data['3']['min']
    mintemp4 = data['4']['min']

    html = '<table align = "center" border = "1">\n' \
           "    <tr>\n" \
           "        <th rowspan = 2>(&deg; F)</th>\n" \
           "        <th>" + str(date0.date()) + "</th>\n" \
           "        <th>" + str(date1.date()) + "</th>\n" \
           "        <th>" + str(date2.date()) + "</th>\n" \
           "        <th>" + str(date3.date()) + "</th>\n" \
           "        <th>" + str(date4.date()) + "</th>\n" \
           "    </tr>\n" \
           "    <tr>\n" \
           "        <th>" + getWeekday(weekday0) + "</th>\n" \
           "        <th>" + getWeekday(weekday0 + 1) + "</th>\n" \
           "        <th>" + getWeekday(weekday0 + 2) + "</th>\n" \
           "        <th>" + getWeekday(weekday0 + 3) + "</th>\n" \
           "        <th>" + getWeekday(weekday0 + 4) + "</th>\n" \
           "    </tr>\n" \
           "    <tr>\n" \
           "        <td> Max Temperature </td>\n" \
           '        <td align="center">' + maxtemp0 + "&deg; F</td>\n" \
           '        <td align="center">' + maxtemp1 + "&deg; F</td>\n" \
           '        <td align="center">' + maxtemp2 + "&deg; F</td>\n" \
           '        <td align="center">' + maxtemp3 + "&deg; F</td>\n" \
           '        <td align="center">' + maxtemp4 + "&deg; F</td>\n" \
           "    </tr>\n" \
           "    <tr>\n" \
           "        <td> Min Temperature </td>\n" \
           '        <td align="center">' + mintemp0 + "&deg; F</td>\n" \
           '        <td align="center">' + mintemp1 + "&deg; F</td>\n" \
           '        <td align="center">' + mintemp2 + "&deg; F</td>\n" \
           '        <td align="center">' + mintemp3 + "&deg; F</td>\n" \
           '        <td align="center">' + mintemp4 + "&deg; F</td>\n" \
           "    </tr>\n" \
           "</table>\n"

    return html


# Make HTML Format for Past Weather
def htmlPast(data):
    date0 = datetime.datetime.strptime(data['0']['date'], "%Y-%m-%d")
    date1 = datetime.datetime.strptime(data['1']['date'], "%Y-%m-%d")
    date2 = datetime.datetime.strptime(data['2']['date'], "%Y-%m-%d")
    date3 = datetime.datetime.strptime(data['3']['date'], "%Y-%m-%d")
    date4 = datetime.datetime.strptime(data['4']['date'], "%Y-%m-%d")

    # Max Temperature
    maxtemp0 = data['0']['TMAX']
    maxtemp1 = data['1']['TMAX']
    maxtemp2 = data['2']['TMAX']
    maxtemp3 = data['3']['TMAX']
    maxtemp4 = data['4']['TMAX']

    # Min Temperature
    mintemp0 = data['0']['TMIN']
    mintemp1 = data['1']['TMIN']
    mintemp2 = data['2']['TMIN']
    mintemp3 = data['3']['TMIN']
    mintemp4 = data['4']['TMIN']

    html = '<table align = "center" border = "1">\n' \
           "    <tr>\n" \
           "        <th>(&deg; F)</th>\n" \
           "        <th>" + str(date4.date()) + "</th>\n" \
           "        <th>" + str(date3.date()) + "</th>\n" \
           "        <th>" + str(date2.date()) + "</th>\n" \
           "        <th>" + str(date1.date()) + "</th>\n" \
           "        <th>" + str(date0.date()) + "</th>\n" \
           "    </tr>\n" \
           "    <tr>\n" \
           "        <td> Max Temperature </td>\n" \
           '        <td align="center">' + str(maxtemp4) + "&deg; F</td>\n" \
           '        <td align="center">' + str(maxtemp3) + "&deg; F</td>\n" \
           '        <td align="center">' + str(maxtemp2) + "&deg; F</td>\n" \
           '        <td align="center">' + str(maxtemp1) + "&deg; F</td>\n" \
           '        <td align="center">' + str(maxtemp0) + "&deg; F</td>\n" \
           "    </tr>\n" \
           "    <tr>\n" \
           "        <td> Min Temperature </td>\n" \
           '        <td align="center">' + str(mintemp4) + "&deg; F</td>\n" \
           '        <td align="center">' + str(mintemp3) + "&deg; F</td>\n" \
           '        <td align="center">' + str(mintemp2) + "&deg; F</td>\n" \
           '        <td align="center">' + str(mintemp1) + "&deg; F</td>\n" \
           '        <td align="center">' + str(mintemp0) + "&deg; F</td>\n" \
           "    </tr>\n" \
           "</table>\n"

    return html


# Make HTML Format for Current Weather
def htmlCurrent(data):
    # Current Weather Icon
    icon = data['weatherIconUrl'][0]['value']
    windspeed = data['windspeedMiles']
    windDegree = data['winddirDegree']
    temp_F = data['temp_F']
    humidity = data['humidity']
    winddirection = data['winddir16Point']
    feelsliketemp_F = data['FeelsLikeF']

    html =  '<table align = "center" border = "1">\n' \
            "   <tr>\n" \
            "       <th> Type </th>\n" \
            "       <th> Value </th>\n" \
            "   </tr>\n" \
            "   <tr>\n" \
            '       <td align = "center">' + 'Temperature' + '</td>\n' \
            '       <td align = "center">' + temp_F + '&deg; F</td>\n' \
            "   </tr>\n" \
            "   <tr>\n" \
            '       <td align = "center">' + 'Humidity' + '</td>\n' \
            '       <td align = "center">' + humidity + '&#37;</td>\n' \
            "   </tr>\n" \
            "   <tr>\n" \
            '       <td align = "center">' + 'Sensible Temperature' + '</td>\n' \
            '       <td align = "center">' + feelsliketemp_F + '&deg; F</td>\n' \
            "   </tr>\n" \
            "   <tr>\n" \
            '       <td align = "center">' + 'Wind Speed' + '</td>\n' \
            '       <td align = "center">' + windspeed + ' mph</td>\n' \
            "   </tr>\n" \
            "   <tr>\n" \
            '       <td align = "center">' + 'Wind Degree' + '</td>\n' \
            '       <td align = "center">' + windDegree + '&deg; </td>\n' \
            "   </tr>\n" \
            "   <tr>\n" \
            '       <td align = "center">' + 'Status' + '</td>\n' \
            '       <td align = "center">\n' \
            '           <img src="' + icon + '" border = "1">\n' \
            '       </td>\n' \
            "   </tr>\n" \
            "</table>\n"

    return html


# Make HTML Format for Indoor Status
def htmlindoor(data):
    humidity = data['H']
    temperature = data['T']
    name = data['Name']

    html = '    <tr>\n' \
           '        <td>' + name + '</td>\n' \
           '        <td>' + str(temperature) + '&deg; F</td>\n' \
           '        <td>' + str(humidity) + '&#37;</td>\n' \
           '    </tr>\n'

    return html

# Get Weekday
def getWeekday(weekday):
    if weekday >= 7:
        weekday = weekday - 7

    if weekday == 0:
        return "Monday"
    if weekday == 1:
        return "Tuesday"
    if weekday == 2:
        return "Wednesday"
    if weekday == 3:
        return "Thursday"
    if weekday == 4:
        return "Friday"
    if weekday == 5:
        return "Saturday"
    if weekday == 6:
        return "Sunday"

################ Program Start #########################
# Get current & forecast data
path_current = "outdoor_current.json"
path_forecast = "outdoor_forecast.json"
path_past = "outdoor_past.json"
path_indoor_dev1 = "indoor_dev1.json"
path_indoor_dev2 = "indoor_dev2.json"
path_zip = "zip_code.json"


# Run the Web Server
site = server.Site(Simple())
reactor.listenTCP(8080, site)

print "Starting Web Server."
print "Server Running..."

reactor.run()

print "Server Stopped..."