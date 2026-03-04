import serial
import struct

s = serial.Serial (port="COM5", baudrate=115200)
s.timeout = 1.0

data = bytes()
while True:
	data = data + s.read(40)

	while len(data) >= 2 and (data[0] != 0xaa or data[1] != 0xaa):
		data = data[1:]

	if len(data) >= 27:
		chk = data[0]
		for b in data[1:27]:
			chk = chk ^ b

		if 0 == chk:
			fstr = "<xxHLHLhhhhhh"
			values = struct.unpack(fstr, data[:26])
			print(values)
			data = data[27:]
		else:
			data = data[1:]