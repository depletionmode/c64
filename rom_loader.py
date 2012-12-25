#I2C ROM Loader script

import struct
import serial
import binascii
import time

with open('', 'rb') as f:
	data = bytearray(f.read())
	
	buf = bytearray(8 + len(data))
	struct.pack_into('>II', buf, 0, addr, len(data))
	
	print binascii.hexlify(buf[:8])
	
	for i in range(len(data)):
		buf[8 + i] = data

	ser = serial.Serial('COM8', 9600)
	time.sleep(2)
	ser.write(data)