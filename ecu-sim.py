import serial
import sys

SETVAL_USAGE = "Usage: setval <key> <value>" 

def error(error_string):
	print error_string
	return

def setValue(connection, parts):
	if (3 < len(parts)):
		return error(SETVAL_USAGE)
	key = int(parts[1])
	val = int(parts[2])
	if(key > 255 or val > 255):
		print "Key of value too high"
		return error(SETVAL_USAGE)
	packet = bytearray()
	packet.append(0x02);
	packet.append(key)
	packet.append(val)
	connection.write(packet)
	resp = connection.read(1)



def setCode(connection, parts):
	print "setting code "

if len(sys.argv) < 2:
	print "Usage: ecu-sim.py <port name>"
	exit()
print "connecting to port", sys.argv[1]
connection = serial.Serial(sys.argv[1], 115200) 

while True:
	line = raw_input(">")	
	parts = line.split()
	if(0 == len(parts)):
		continue
	if "setval" == parts[0].lower():		
		setValue(connection, parts)
	elif "setcode" == parts[0].lower():
		setCode(connection, parts)
	else:
	    print "malformed command:" + line