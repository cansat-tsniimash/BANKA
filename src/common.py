import struct

def parse_one(chunk: bytes):
	data = chunk
	while True:
		while len(data) >= 2 and (data[0] != 0xaa or data[1] != 0xaa):
			data = data[1:]

		if len(data) < 56:
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
			print("checksum error")
			data = data[1:]


def convert(values: tuple):
	return (
	 values[0], #team id
	 values[1], #время, мс
	 values[2]/100, # температура, градусы
	 values[3], # Давление, Па
	 lsm6ds3_from_fs16g_to_mg(values[4]), # ускорение по X на 16г
	 lsm6ds3_from_fs16g_to_mg(values[5]), # ускорение по У на 16г
	 lsm6ds3_from_fs16g_to_mg(values[6]), # ускорение по Z на 16г
	 lsm6ds3_from_fs2000dps_to_mdps(values[7]), # угловая скорость по X на 2000 градусов
	 lsm6ds3_from_fs2000dps_to_mdps(values[8]),# угловая скорость по У на 2000 градусов
	 lsm6ds3_from_fs2000dps_to_mdps(values[9])# угловая скорость по Z на 2000 градусов
)


def lsm6ds3_from_fs2g_to_mg(lsb: int) -> float:
  return lsb * 61.0 / 1000.0

def lsm6ds3_from_fs4g_to_mg(lsb: int) -> float:
  return lsb * 122 / 1000.0

def lsm6ds3_from_fs8g_to_mg(lsb: int) -> float:
  return lsb * 244.0 / 1000.0

def lsm6ds3_from_fs16g_to_mg(lsb: int) -> float:
  return lsb * 488.0 / 1000.0

def lsm6ds3_from_fs125dps_to_mdps(lsb: int) -> float:
  return lsb * 4375.0 / 1000.0

def lsm6ds3_from_fs250dps_to_mdps(lsb: int) -> float:
  return lsb * 8750.0 / 1000.0

def lsm6ds3_from_fs500dps_to_mdps(lsb: int) -> float:
  return lsb * 1750.0 / 1000.0

def lsm6ds3_from_fs1000dps_to_mdps(lsb: int) -> float:
  return lsb * 35.0

def lsm6ds3_from_fs2000dps_to_mdps(lsb: int) -> float:
  return lsb * 70.0