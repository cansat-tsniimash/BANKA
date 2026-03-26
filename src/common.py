import struct

def parse_one(chunk: bytes):
	data = chunk
	while True:
		while len(data) >= 2 and (data[0] != 0xaa or data[1] != 0xaa):
			data = data[1:]

		if len(data) < 27:
			return None, data

		chk = data[0]
		for b in data[1:27]:
			chk = chk ^ b

		if 0 == chk:
			fstr = "<xxHLHLhhhhhh"
			values = struct.unpack(fstr, data[:26])
			data = data[27:]
			return values, data
		else:
			data = data[1:]
