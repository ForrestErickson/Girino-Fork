#! /usr/bin/env python
# -*- coding: UTF-8 -*-
# This appears to be written for Pyton 2.
# Changing to Python 3 
# Forrest Lee Erickson
# Date: 20210203

import serial
from struct import unpack
import numpy as np

#stream = serial.Serial( '/dev/tty.usbmodem411', 115200 )  # open first serial port
stream = serial.Serial( 'COM8', 115200 )  # open ARDUINO serial port
print(stream.portstr)       # check which port was really used
line = stream.readline()   # read a '\n' terminated line
print("Line is: " + str(line))
#print(line.find("Girino ready" ))

LOW = 0
HIGH = 1

threshold = 256/2

#if ( (line.find( "Girino ready" )) != -1 ):
if ( 1 ):
#if ( str.find(line, "Girino ready" ) != -1 ):

	print("Girino tells me that it is ready")

#	stream.write('d')
	stream.write(bytes('d', 'utf-8'))

	for i in range(6):
		print(stream.readline(), "utf-8")

	stream.write(bytes('p128', 'utf-8'))
	print(stream.readline())

	stream.write(bytes('t50', 'utf-8'))
	print(stream.readline())

#Let's print error message.
	stream.write(bytes('x', 'utf-8'))
	print(stream.readline())

#Let's get and print status
	stream.write(bytes('d', 'utf-8'))

	for i in range(6):
		print(stream.readline(), "utf-8")

	print("Start A2D and get eventData")

	eventData = []

	for i in range(5):
		stream.write(bytes('s', 'utf-8'))
		eventString = stream.read(1280)
		eventData.append(np.array(unpack( '1280B', eventString )))
		print( "eventData: " )
		print( eventData )
		stream.write(bytes('S', 'utf-8'))
		stream.flushInput()

	np.savetxt( "data.txt", eventData[4] )

	i = 0
	status = LOW
	delta = []   
    

	for datum in eventData[4]:
		if datum > threshold and status == LOW:
			status = HIGH
			pre = i
		if datum < threshold and status == HIGH:
			status = LOW
			delta.append(i - pre)
		i += 1

	np.savetxt( "deltas.txt", delta )
			
		
stream.flushOutput()
stream.flushInput()
stream.close()             # close port
