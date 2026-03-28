import serial
import struct
from common import parse_one, convert

s = serial.Serial (port="COM5", baudrate=115200)
s.timeout = 1.0
f = open("text.data", "wb")
csv = open("file.csv", mode = "w")

accum = b""
while True:
	chunk = s.read(1024)
	f.write(chunk)
	f.flush()

	accum = accum + chunk
	while len(accum) > 27:
		values, leftovers = parse_one(accum)
		numbers = convert(values)

		print(numbers)
		csv.write(";".join([str(x).replace(".", '.') for x in numbers])+ "\n")
		accum = leftovers
