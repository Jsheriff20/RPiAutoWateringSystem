# connect to database
# in a true loop:
#     check if the moisture value is beneth the minimum
#     if it is, use paho to publish a message ("the amount to water the plant") to a topic on MQTT
#     wait 30 minutes

import time
import mysql.connector
import paho.mqtt.client as mqtt    #import client library

#mqtt broker disconnect and connect events
def onConnect(client, userData, flags, returnCode):
    if returnCode==0:
        print("connected")
    else:
        print("failed to connect - Returned code:", returnCode)
def onDisconnect(client, userData, flags, returnCode=0):
    print("Disconnected result code: " + str(returnCode))



#connect to mysql database
conn = mysql.connector.connect(host="aauzw8rq4r6wmc.cywl2p0cmnmu.us-east-1.rds.amazonaws.com",
                               user="admin",
                               password="Password123",
                               database="ebdb")
c = conn.cursor()

while True:
    #check current/ last reading of mositure sensor
    c.execute("SELECT sensorReading FROM moistureSensor ORDER BY unixTime DESC LIMIT 1 ")
    sensorValue = c.fetchone()
    sensorValue = sensorValue[0]

    #get users set values
    c.execute("SELECT minimumMoisture, wateringAmount FROM wateringInfo WHERE gpio = 1")
    results = c.fetchall()
    minimumValue = results[0][0]
    wateringAmount = results[0][1]

    #check if moisture level is too low
    try:
        if(int(sensorValue) <= int(minimumValue)):
            #connect to broker
            broker="172.31.95.84"
            client = mqtt.Client("moistureChecker")
            client.username_pw_set("admin", "Password123")
            client.tls_set("/home/ubuntu/cert/ca.crt")
            client.tls_insecure_set(True)
            client.on_connect = onConnect
            client.on_disconnect = onDisconnect

            print("Connecting to broker")

            #publish message to topic "moistureChecker" on broker
            client.connect(broker,port=8883)
            client.loop_start()

            client.publish("moistureChecker", wateringAmount)
            print("Published")
            time.sleep(3)
            client.loop_stop()
            client.disconnect()
    except Exception as ex:
        print("log error - ", ex)
        # if(minimumValue == "N/A"):
        #     print("log Value is N/A")
        # else:
        #     print("log error - ", ex)
    #wait 30 mins
    time.sleep(1800)

conn.close()

