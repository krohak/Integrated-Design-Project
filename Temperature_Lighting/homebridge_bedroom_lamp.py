import tornado.ioloop
import tornado.web
import serial
import requests
import json
import re

port = "/dev/ttyACM0"
serialArduino = serial.Serial(port,9600)
serialArduino.flushInput()

lamp = 0
br = 50
target = 23
#msg=serialArduino.readline()

def lamp_on():
	print("Lamp is turned ON")
	lamp = 1
	serialArduino.write("n"+str(br))


def lamp_off():
	print("Lamp is turned OFF")
	lamp = 0
	serialArduino.write("n0")



def read_lamp():
	return str(lamp)

def lamp_brightness(percent):
	global br
	br=percent
	serialArduino.write("n"+str(percent))
	#print(br)
	#dta = json.loads(self.request.body)


def read_brightness():
        #return str(lamp)
	return str(br)

def read_weather():
        #temp=sense.get_temperature()
        #hum=sense.get_humidity()
        temp=27
        hum=45
	weather={ "targetHeatingCoolingState":3,"targetTemperature":target,"targetRelativeHumidity":55.0, "currentHeatingCoolingState":2, "currentTemperature": temp, "currentRelativeHumidity": hum, "getTemperatureDisplayUnits": 1}
        #weather={ "temperature": temp,"humidity": hum }
        return(weather)

def set_temp(level):
	global target
	target=level
	print("t"+str(level))
	serialArduino.write("t"+str(level))


class LampHandler(tornado.web.RequestHandler):
	def get(self, action):
		try:
			if action == "/lamp/on":
				# /lamp/on
				lamp_on()
			elif action == "/lamp/off":
				# /lamp/off
				lamp_off()
			elif action == "":
				# /lamp
				self.write(read_lamp())
			#elif action == "/lamp/brightness(.*)":
				#lamp_brightness(self)
			elif action == "/lamp/100054":
				self.write(read_brightness())
			#elif action == "/weather/status":
		except tornado.httpclient.HTTPError as e:
			print("Error: " + str(e))
		except Exception as e:
			print("Error2: " + str(e))
	

class BrightnessHandler(tornado.web.RequestHandler):
	try:	
		def get(self,percent):
			try:
				#dat=(self.request.url)
				#print(url)
				lamp_brightness(percent)
			except Exception as e:
				print("hi")
				print(e)

	except Exception,e:
		print("hello4")
		print(e)


class TempHandler(tornado.web.RequestHandler):
	def get(self,action):
        	if action=="/status":                
			try:
                            	self.write(read_weather())			
                        except Exception as e:
                                print("hi")
                                print(e)
		
		elif re.search("/targetTemperature/(\d+)",action):
			print("re found")
			print(action)
			level=action.split("/")[2]
			set_temp(level)
			
		
		elif action=="/off":
			print("off")
			serialArduino.write("t"+str(29))
			#set_temp(29)

		elif action=="/auto":
			print("auto")
			set_temp(target)
	
		else:
			print("ok1")
			print(action)
		
		




def make_app():
	return tornado.web.Application([
		(r"/bedroom(.*)", LampHandler),
		(r"/brightness/(\d+)",BrightnessHandler),
		(r"/weather(.*)",TempHandler),
	])

if __name__ == "__main__":
	app = make_app()
	app.listen(80)
	tornado.ioloop.IOLoop.current().start()




