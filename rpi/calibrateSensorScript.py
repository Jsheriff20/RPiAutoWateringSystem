import smbus 
import time

address = 0x48
A0 = 0x40

bus = smbus.SMBus(1)
highestValue = 0
lowestValue = 1000

while True:
	bus.write_byte(address, A0)
	value = bus.read_byte(address)

	if(value < lowestValue):
		lowestValue = value
	elif(value > highestValue):
		highestValue = value
	print("Highest Value: " + str( highestValue))
	print("LowestValue: " + str(lowestValue))
	time.sleep(1)
