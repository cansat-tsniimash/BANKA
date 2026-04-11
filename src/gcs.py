import serial
import struct
from common import parse_one, convert

s = serial.Serial (port="COM5", baudrate=115200)
s.timeout = 1.0
f = open("text.data", "wb")
csv = open("file.csv", mode = "w")
while True:
	chunk = s.read(1024)
	print(chunk)
	f.write(chunk)
	f.flush()

	while len(chunk) > 27:
		values, leftovers = parse_one(chunk)
		numbers = convert(values)
		print(numbers)
		csv.write(";".join([str(x).replace(".", '.') for x in numbers])+ "\n")
		chunk = leftovers
