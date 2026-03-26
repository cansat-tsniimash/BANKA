import serial
import struct
from common import parse_one

s = serial.Serial (port="COM5", baudrate=115200)
s.timeout = 1.0
f = open("text.data", "wb")

accum = b""
while True:
	chunk = s.read(1024)
	f.write(chunk)
	f.flush()

	accum = accum + chunk
	while len(accum) > 27:
		values, leftovers = parse_one(accum)
		print(values, len(leftovers))
		accum = leftovers
