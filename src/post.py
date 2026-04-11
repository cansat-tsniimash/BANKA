
from common import parse_one, convert

f = open("example.data", "rb")

accum = b"" # создаю пустой коллаж с типом данным в нём
while True: # бесконечный цикл
	chunk = f.read(1024) # читает 1024 бита из файла
	if not chunk:
		break

	accum = accum + chunk
	#print("".join("%02X" % x for x in accum))
	while len(accum) > 27:
		values, leftovers = parse_one(accum)
		accum = leftovers

		if values is not None:
			numbers = convert(values)
			print("{:<8} {:<11} {:<13} {:<11} {:<11} {:<11} {:<11} {:<11} {:<11} {:<11}".format(
				  "team id", "t", "degr", "pa", "aX", "aY", "aZ", "w speedX", "w speedY", "w speedZ",
			))
			print("{:<8} {:<11} {:<13} {:<11} {:<11} {:<11} {:<11} {:<11} {:<11} {:<11}".format(
				*numbers
			))
			print("-" * 115)
			

			