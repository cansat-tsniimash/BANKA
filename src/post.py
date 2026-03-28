from common import parse_one

f = open("text.data", "rb")

while True:
	chunk = s.read(40)
	values = parse_one(chunk)
	print(values)

