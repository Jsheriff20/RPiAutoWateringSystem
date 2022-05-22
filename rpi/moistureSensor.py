import smbus
import time
import mysql.connector


conn = mysql.connector.connect(host="aauzw8rq4r6wmc.cywl2p0cmnmu.us-east-1.rds.amazonaws.com",
				user="admin",
				password="Password123", 
				database="ebdb")
c = conn.cursor()


address = 0x48
A0 = 0x40

bus = smbus.SMBus(1)

dryNum = 119
fullWetNum = 55
range = dryNum - fullWetNum

bus.write_byte(address, A0)
value = bus.read_byte(address)

percentage = ((value - fullWetNum) * 100)/(range)
print(percentage)
c.execute("INSERT  INTO moistureSensor (sensorReading, unixTime) VALUES(%s, %s)",
	(percentage,time.time()))

conn.commit()
conn.close()
