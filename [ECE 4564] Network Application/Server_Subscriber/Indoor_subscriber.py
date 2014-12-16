__author__ = 'Yosub Lee'

"""
    Team Raven - Personal Weather Station 'Indoor'
    This program gets messages(data) through RabbitMQ

    Date    : 12/1/2014
"""
import pika
import json
import datetime
import time
from pprint import pprint
# Global Variables
running = True
port = 5672
topic = "Indoor.data"
routing_key = "Indoor"
host = 'localhost'

counter = 0
time_counter = 0
records = dict()


def toggleLED(pinNum):
    """
    Toggle Pin to control LED.
    :param pinNum: GPIO Pin Number
    :return: None
    """
    # Do something

def on_new_msg(channel, delivery_info, msg_properties, msg):
    """
    Event handler that processes new messages from the message broker

    For details on interface for this pika event handler, see:
    https://pika.readthedocs.org/en/0.9.14/examples/blocking_consume.html

    :param channel: (pika.Channel) The channel object this message was received
                 from
    :param delivery_info: (pika.spec.Basic.Deliver) Delivery information related
                       to the message just received
    :param msg_properties: (pika.spec.BasicProperties) Additional metadata about
                        the message just received
    :param msg: The message received from the server
    :return: None
    """
    try:
        global counter
        global records
        global time_counter

        if msg is None:
            print "Msg is Empty"
        else:
            stats = json.loads(msg)
            print "received: ", stats
            filename_current = "indoor_{}.json".format(stats["Name"])
            filename_accum = "indoor_{}_record.json".format(stats["Name"])
            print 'file: ', filename_current
            output = open(filename_current, 'w')

            stats["Time"] = time.strftime("%H:%M")
            stats["Date"] = time.strftime("%d/%m/%Y")

            # indoor_dev1_current
            json.dump(stats, output)

            # Every 10 min. It collects Data. (For Graphing)
            if time_counter > 8:
                print "Graph"
                # haha..it is ugly...
                if counter < 24:
                    records[counter] = stats
                    counter += 1
                else:
                    for x in xrange(0, 23):
                        records[x] = records[x+1]

                    records[23] = stats

                output_record = open(filename_accum, 'w')
                json.dump(records, output_record)
                output_record.close()
                time_counter = 0
            else:
                output.close()
                time_counter += 1

    except ValueError, e:
        print "Message Cannot be parsed = ", e


"""
[ Main Program }
"""
# initalize dictionary object
for x in xrange(0, 24):
    records[x] = {"Date": "00/00/0000", "H": "0.00", "Time":"00:00", "T":"0.00", "Name":"dev0"}

message_broker = None
channel = None

message_broker = pika.BlockingConnection(pika.ConnectionParameters(host=host,
                                                                  port=port))
channel = message_broker.channel()
channel.exchange_declare(exchange=topic,
                         type='topic')

result = channel.queue_declare(exclusive=True)
queue_name = result.method.queue

channel.queue_bind(exchange=topic, queue=queue_name, routing_key=routing_key)

print "Waiting..."

channel.basic_consume(on_new_msg,
                      queue=queue_name,
                      no_ack=True)

channel.start_consuming()
