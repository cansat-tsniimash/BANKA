/*
 * appmain.c
 *
 *  Created on: Feb 1000-7, 2026
 *      Author: nikit
 */
#include "stm32f1xx.h"
#include "ds18b20/ds18b20.h"
#include "neo6mv2/neo6mv2.h"
#include "bmp280/bmp280.h"
#include "lsm6ds3/lsm6ds3.h"
#include "lis2mdl/lis2mdl.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "lora/e220.h"
#include "algoritm.h"
#include "photorez/photorez.h"


extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
#pragma pack(push, 1)
typedef struct {
	uint16_t start;
	uint16_t team_id;
	uint32_t time;
	int16_t temperature;
	uint32_t pressure;
	int16_t acc[3];
	int16_t gyro[3];
	uint8_t summ;

	uint16_t pocket_number;
	float shortness_gps;
	float longitude_gps;
	float height_gps;
	uint8_t fix_gps;
	int16_t temp_ds18b20;
	int16_t magn[3];
	uint16_t photorez;
	uint8_t state;
	uint8_t banka_summ;
}packet_t;

#pragma pack(pop)

uint8_t checksum(const void * data_, size_t data_size)
{
	const uint8_t *data = (const uint8_t*)data_;
	if (data_size == 0)
		return 0;

	uint8_t c = data[0];
	for (size_t i = 1; i < data_size; i++)
		c = c ^ data[i];

	return c;
}

#define BMP280_ADDR (0x76 << 1)

void app_main(void)
{
	packet_t packet = {0};
	packet.start = 0xAAAA;
	packet.team_id = 0xBBBB;

	ds18b20_init(DS18B20_12_BIT);
	uint32_t ds_start_time = HAL_GetTick();
	ds18b20_conv();

	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_ERR);
	neo6mv2_Init();
	GPS_Data gps_data;


	struct bme280_dev bmp280;
	bmp280.intf = BME280_I2C_INTF;
	bmp280.read = bmp280_read_reg;
	bmp280.write = bmp280_write_reg;
	bmp280.delay_us = bmp280_delay_us;

	bmp280.settings.osr_p = BME280_OVERSAMPLING_16X;
	bmp280.settings.osr_t = BME280_OVERSAMPLING_16X;
	bmp280.settings.filter = BME280_OVERSAMPLING_16X;
	bmp280.settings.standby_time = BME280_STANDBY_TIME_20_MS;

	bmp280_bus_t bmp_bus;
	bmp_bus.ADDR = 0x76 << 1;
	bmp_bus.hi2c = &hi2c1;
	bmp280.intf_ptr = &bmp_bus;

	bme280_init(&bmp280);
	bme280_set_sensor_settings(BME280_ALL_SETTINGS_SEL, &bmp280);
	bme280_set_sensor_mode(BME280_NORMAL_MODE, &bmp280);
	struct bme280_data bmp_data;

	lsm_data_t lsm_bus;
	lsm_bus.ADDR = 0x6A << 1;
	lsm_bus.hi2c = &hi2c1;

	stmdev_ctx_t lsm6ds3;
	lsm6ds3.handle = &lsm_bus;
	lsm6ds3.write_reg = lsm_write_reg;
	lsm6ds3.read_reg = lsm_read_reg;

	lsm6ds3_reset_set(&lsm6ds3, 1);
	lsm6ds3_xl_full_scale_set(&lsm6ds3, LSM6DS3_16g);
	lsm6ds3_xl_data_rate_set(&lsm6ds3, LSM6DS3_XL_ODR_104Hz);
	lsm6ds3_gy_full_scale_set(&lsm6ds3, LSM6DS3_2000dps);
	lsm6ds3_gy_data_rate_set(&lsm6ds3, LSM6DS3_GY_ODR_208Hz);

	int16_t buf_lsm_gy[3] = {0};
	int16_t buf_lsm_xl[3] = {0};
	volatile float gyro[3] = {0};
	volatile float acc[3] = {0};

	lis_data_t lis_bus;
	lis_bus.ADDR = 0x1E << 1;
	lis_bus.hi2c = &hi2c1;

	lis2mdl_ctx_t lis;
	lis.handle = &lis_bus;
	lis.write_reg = lis_write_reg;
	lis.read_reg = lis_read_reg;

	lis2mdl_reset_set(&lis, 1);
	lis2mdl_operating_mode_set(&lis, LIS2MDL_CONTINUOUS_MODE);
	lis2mdl_data_rate_set(&lis, LIS2MDL_ODR_50Hz);
	lis2mdl_power_mode_set(&lis, LIS2MDL_HIGH_RESOLUTION);


	  e220_connect_t e220;
	  e220.uart = &huart2;
	  e220.aux_pin = GPIO_PIN_3;
	  e220.aux_port = GPIOB;
	  e220.m0_pin = GPIO_PIN_1;
	  e220.m0_port = GPIOB;
	  e220.m1_pin = GPIO_PIN_0;
	  e220.m1_port = GPIOB;

	  e220_mode_switch(&e220, E220_MODE_DSM);
	  HAL_Delay(200);
	  e220_set_channel(&e220, 1);
	  HAL_Delay(200);
	  e220_set_add(&e220, 43690);
	  HAL_Delay(300);
	  e220_reg_0(&e220, E220_AIR_RATE_9P6, E220_SERIAL_PARITY_BIT_8N1, E220_SERIAL_PORT_RATE_9600);
	  HAL_Delay(200);
	  e220_reg_1(&e220, E220_SUB_PACKET_SETTING_200B, E220_RSSI_AMBIENT_NOICE_DISABLE, E220_TRANSMITTING_POWER_10DBM);
	  HAL_Delay(200);
	  e220_mode_switch(&e220, E220_MODE_TM);

	int16_t buf_lis[3] = {0};
	volatile float magn[3] = {0};

	FATFS sd;
	FIL packet1;
	char packet1_path[] = "0:/pocket1.bin";
	FRESULT result_mount = f_mount(&sd, "0:", 1);

	//f_mount(&sd, "1:", 1);

	FRESULT rezult_pocket1 = 255;
	UINT byte_count;
	CanSatState_t state_now = STATE_IN_ROCKET;
	uint32_t state_timer = 0;
	bme280_get_sensor_data(BME280_TEMP | BME280_PRESS, &bmp_data, &bmp280);
	uint32_t first_pressure = bmp_data.pressure;

	float photorez_data;

	uint16_t raw_adc_value;

	while(1)
	{
		uint16_t raw_adc_value = 0;
		HAL_ADC_Start(&hadc1);
		HAL_ADC_GetValue(&hadc1);
		packet.photorez = raw_adc_value;

		/*HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
		{
		    raw_adc_value = HAL_ADC_GetValue(&hadc1);
		}
		if (raw_adc_value < 1000) {
		HAL_ADC_Start(&hadc1);
		   	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		}
		else
		{
				HAL_ADC_Start(&hadc1);
		    	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		}*/

		bme280_get_sensor_data(BME280_TEMP | BME280_PRESS, &bmp_data, &bmp280);
		packet.pressure = bmp_data.pressure;
		packet.temperature = bmp_data.temperature * 100;
		float altitude = 44330 * (1 - pow((float)bmp_data.pressure / first_pressure, (1.0 / 5.255)));

		lsm6ds3_acceleration_raw_get(&lsm6ds3, buf_lsm_xl);
		lsm6ds3_angular_rate_raw_get(&lsm6ds3, buf_lsm_gy);
		for (int i = 0; i < 3; i++)
		{
			packet.acc[i] = buf_lsm_xl[i];
			packet.gyro[i] = buf_lsm_gy[i];
		}

		lis2mdl_magnetic_raw_get(&lis, buf_lis);
		for (int i = 0; i < 3; i++)
		{
			magn[i] = lis2mdl_from_lsb_to_mgauss(buf_lis[i]) /1000.0;
		}

		for (int i = 0; i <= 10; i++)
		{
			if (neo6mv2_work() == 1)
				break;
		}
		gps_data = neo6mv2_GetData();


		if ((HAL_GetTick() - ds_start_time) > 750)
		{
			volatile float temp = ds18b20_readtemp();
			ds18b20_conv();
			ds_start_time = HAL_GetTick();

		}


		switch (state_now)
		{
		case STATE_INIT:
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET)//таймер 15 секунд после перекл. состояния
			{
				if (HAL_GetTick() - state_timer > 15000)
				{
					photorez_data = photorez_read_data();
					state_now = STATE_IN_ROCKET;
				}
			}
			else
			{
				state_timer = HAL_GetTick();
			}
			break;
		case STATE_IN_ROCKET:
			if (photorez_read_data() >= photorez_data)//фоторезистор
				{
				state_now = STATE_FLIGHT;
				}
			break;
//		case STATE_IN_ROCKET:
//			if (altitude <= 100)
//			{
//				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//			}
//			break;
//		case STATE_BB_SEPARATE:
//			if ()//вкл_нагреватель_на_10с  пережигатель_вкл
//			{
//				пережигатель_вкл
//				state_now = SATE_ON_GROUND;
//			}
//			break;
//		case SATE_ON_GROUND:
//			if ()//пережигатель_выкл
//			{
//			пищалка_вкл
//			}
//			break;
		}



		packet.pocket_number += 1;
		packet.time = HAL_GetTick();
		packet.summ = checksum(&packet, offsetof(packet_t, summ));
		packet.banka_summ = checksum(&packet, offsetof(packet_t, banka_summ) - offsetof(packet_t, pocket_number));


		//		uint8_t seq[128];
		//		for (size_t i = 0; i < sizeof(seq); i++)
		//			seq[i] = i;

		e220_send_packet(&e220, (uint8_t *)&packet, sizeof(packet_t));
		//e220_send_packet(&e220, seq, sizeof(seq));


		if (result_mount != FR_OK)
		{
			f_mount (NULL, "", 1);
			extern Disk_drvTypeDef disk;
			disk.is_initialized[0] = 0;
			result_mount = f_mount(&sd, "", 1);
		}

		if (result_mount == FR_OK && rezult_pocket1 != FR_OK)
		{
			if (rezult_pocket1 != 255)
				f_close(&packet1);
			rezult_pocket1 = f_open(&packet1, (const TCHAR*) &packet1_path, FA_WRITE | FA_OPEN_ALWAYS | FA__WRITTEN);
			if (rezult_pocket1 != FR_OK)
				result_mount = 255;

		}

		if (rezult_pocket1 == FR_OK && result_mount == FR_OK)
		{
			rezult_pocket1 = f_write(&packet1, &packet, sizeof(packet_t), &byte_count);
			rezult_pocket1 = f_sync(&packet1);
		}

		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);*/
	}
	return;
}
