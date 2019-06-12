import re
import serial
import sys

SETVAL_USAGE = "Usage: setval <key> <value>" 
SETCODE_USAGE = "Usage: setcode <code>"

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
	packet.append(0x02)
	packet.append(key)
	packet.append(val)
	connection.write(packet)
	resp = connection.read(1)



def setCode(connection, parts):
	if (2 < len(parts)):
		return error(SETCODE_USAGE)
	code = parts[1].upper()
	x = re.search("^[PCBU][0-3]{1}[0-9A-F]{3}$", parts[1])
	if x is None:
		return error("Bad code \"" + parts[1] + "\"");	
	if 'P' == code[0]:
	    byte1 = 0x00
	elif 'C' == code[0]:
		byte1 = 0x40
	elif 'B' == code[0]:
		byte1 = 0x80
	elif 'U' == code[0]:
		byte1 = 0xC0

	byte1 = byte1 | (int(code[1]) << 4)
	byte1 = byte1 | int(("0" + code[2]), 16)
	byte2 = int(("" + code[3] + code[4]), 16)
	packet = bytearray()
	packet.append(0x03)
	packet.append(byte1)
	packet.append(byte2)
	connection.write(packet)
	resp = connection.read(1)
	

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