#include "nrf.h"
#include "app_error.h"
#include "bsp.h"
#include "nrf_delay.h"
//#include "nrf_pwm.h"
//#include "nrf_drv_pwm.h"
#include "nrf_log.h"
//#include "nrf_drv_config.h"
#include "nrf_drv_twi.h"
#include "BNO055.h"

static nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(0);

void init_twi() 
{
		nrf_drv_twi_config_t config = {
				.scl				= PIPAS_SCL_PIN,
				.sda				= PIPAS_SDA_PIN,
				.frequency	= NRF_TWI_FREQ_100K
		};
		APP_ERROR_CHECK(nrf_drv_twi_init(&twi, &config, NULL, NULL));
		nrf_drv_twi_enable(&twi);
}

void BNO055_select_register_map_page(uint8_t page) 
{
		static uint8_t cmd[2];
		cmd[0] = PAGE_ID_REGISTER;
		cmd[1] = page;
		APP_ERROR_CHECK(nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 2, false));
}

void BNO055_set_power_mode()
{
	uint8_t cmd[2] = {PWR_MODE_REGISTER, PWR_MODE_NORMAL};
	APP_ERROR_CHECK(nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 2, false));
}

void BNO055_select_units()
{
		BNO055_select_register_map_page(0);
		uint8_t cmd[2] = {UNIT_SELECTION_REGISTER, UNIT_ORI_WIN + UNIT_ACC_MSS + UNIT_GYR_DPS + UNIT_EULER_DEG + UNIT_TEMP_C};
		//uint8_t cmd[2] = {UNIT_SELECTION_REGISTER, UNIT_ACC_MSS + UNIT_EULER_DEG};
		APP_ERROR_CHECK(nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 2, false));
}

void BNO055_select_ndof_fusion() 
{
		BNO055_select_register_map_page(0);
		uint8_t cmd[2] = { OPR_MODE_REGISTER, IMU_FUSION_MODE };
		APP_ERROR_CHECK(nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 2, false));
}

void BNO055_select_internal_oscillator() 
{
		BNO055_select_register_map_page(0);
		uint8_t cmd[2] = {SYS_TRIGGER, 0};
		APP_ERROR_CHECK(nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 2, false));
}

bool BNO055_check_device_id() 
{
		//BNO055_select_register_map_page(0);
		bool b_ok = true;
		
		static uint8_t cmd[7];
		cmd[0] = 0;
		cmd[1] = 0;
		cmd[2] = 0;
		cmd[3] = 0;
		cmd[4] = 0;
		cmd[5] = 0;
		cmd[6] = 0;
		
		uint8_t error_code;
		//APP_ERROR_CHECK(nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 1, false));
		//APP_ERROR_CHECK(nrf_drv_twi_rx(&twi, BNO055_I2C_ADDRESS, &cmd[1], 7));

		error_code = nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 1, false);
		//printf("Error code = %d", error_code);
		NRF_LOG_PRINTF("Error code = %d", error_code);
	
		error_code = nrf_drv_twi_rx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 7);
		NRF_LOG_PRINTF("Error code = %d", error_code);
	
		char* status = "test";
	
		if (cmd[0] != BNO055_CHIP_ID) {
				b_ok = false;
		}
		if (cmd[1] != BNO055_ACC_ID) {
				b_ok = false;
		}
		if (cmd[2] != BNO055_MAG_ID) {
				b_ok = false;
		}
		if (cmd[3] != BNO055_GYRO_ID) {
				b_ok = false;
		}
		return b_ok;
}

uint8_t BNO055_calibration_status() 
{
		BNO055_select_register_map_page(0);
		uint8_t cmd[1] = {CALIBRATION_STATUS};

		APP_ERROR_CHECK(nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 1, false));
		APP_ERROR_CHECK(nrf_drv_twi_rx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 1));
		
		return cmd[0];
}

void BNO055_init() 
{
		init_twi();
		nrf_delay_ms(30);
		BNO055_check_device_id();
		nrf_delay_ms(30);
		BNO055_set_power_mode();
		nrf_delay_ms(30);
		BNO055_select_internal_oscillator();
		nrf_delay_ms(30);
		BNO055_select_units();
		nrf_delay_ms(30);
		BNO055_select_ndof_fusion();
		nrf_delay_ms(30);
}
	
void BNO055_read_euler_angles(double* _heading, double* _roll, double* _pitch)
{
		uint8_t cmd[6] = {EULER_H_REGISTER_LSB, 0, 0, 0, 0, 0};
		uint16_t heading;
		uint16_t roll;
		uint16_t pitch;
		BNO055_select_register_map_page(0);
		nrf_delay_ms(10);
		APP_ERROR_CHECK(nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 1, false));
		APP_ERROR_CHECK(nrf_drv_twi_rx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 6));
		
		//heading = cmd[1] << 8 | cmd[0];
		//pitch = cmd[3] << 8 | cmd[2];
		//roll = cmd[5] << 8 | cmd[4];
		
		heading = ((uint16_t)cmd[0]) | (((uint16_t)cmd[1]) << 8);
		pitch = ((uint16_t)cmd[2]) | (((uint16_t)cmd[3]) << 8);
		roll = ((uint16_t)cmd[4]) | (((uint16_t)cmd[5]) << 8);
		
		//*_heading = (double) (heading >> 4);
		//*_roll = (double) (roll >> 4);
		//*_pitch = (double) (pitch >> 4);
		
		*_heading = ((double)heading) / 16.0;
		*_roll = ((double)roll) / 16.0;
		*_pitch = ((double)pitch) / 16.0;
}

void BNO055_read_accel(double *_x, double *_y, double *_z)
{
	uint8_t cmd[6] = {ACCEL_X_REGISTER_LSB, 0, 0, 0, 0, 0};
	uint16_t x;
	uint16_t y;
	uint16_t z;
	BNO055_select_register_map_page(0);
	nrf_delay_ms(10);
	APP_ERROR_CHECK(nrf_drv_twi_tx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 1, false));
	APP_ERROR_CHECK(nrf_drv_twi_rx(&twi, BNO055_I2C_ADDRESS, &cmd[0], 6));
	
	x = ((int16_t)cmd[0]) | (((int16_t)cmd[1]) << 8);
  y = ((int16_t)cmd[2]) | (((int16_t)cmd[3]) << 8);
  z = ((int16_t)cmd[4]) | (((int16_t)cmd[5]) << 8);
	
	*_x = ((double)x) / 100.0;
	*_y = ((double)y) / 100.0;
	*_z = ((double)y) / 100.0;
}
