#!/usr/bin/python

import serial
import requests
import base64
import time
import os

port = "/dev/ttyACM0"
t1=time.time()

serialFromArduino = serial.Serial(port,9600)
serialFromArduino.flushInput()
while True:
	if (serialFromArduino.inWaiting() > 0):
		flag=str(serialFromArduino.read(2))
		if(flag=="ys"):
			print("ys")
			input = str(serialFromArduino.read(1))
			img_data=""
			while(input!="!"):
				img_data+=str(input)
				print(input)
				input = str(serialFromArduino.read(1))
			print(img_data)
			f = open('IMAGE.JPG', 'w')
			f.truncate()
			f.write(img_data.decode('base64'))
			f.close()
			t1=time.time()
			print("done")
		elif(flag=="No"):
			print("No")
			print(((time.time())-t1))
			if((time.time()-t1)>30):
				try:
					os.remove('IMAGE.JPG')
					t1=time.time()
					print("deleted")
				except Exception,e:
					print("no file to be deleted")
