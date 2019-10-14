#include "nrf.h"
#include "app_error.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "nrf_log.h"
//#include "nrf_drv_config.h"
#include "nrf_twi.h"

#define CHIP_ID_REGISTER 					0x00
#define PAGE_ID_REGISTER					0x07
#define OPR_MODE_REGISTER					0x3d
#define PWR_MODE_REGISTER					0x3e
#define SYS_TRIG_REGISTER					0x3f
#define UNIT_SELECTION_REGISTER		0x3b

#define PWR_MODE_NORMAL						0x00
#define PWR_MODE_LOW							0x01
#define PWR_MODE_SUSPEND					0x02
/* Accel data register */
    
#define ACCEL_X_REGISTER_LSB 			0X08
#define ACCEL_X_REGISTER_MSB 			0X09
#define ACCEL_Y_REGISTER_LSB 			0X0A
#define ACCEL_Y_REGISTER_MSB 			0X0B
#define ACCEL_Z_REGISTER_LSB 			0X0C
#define ACCEL_Z_REGISTER_MSB 			0X0D
		
#define EULER_H_REGISTER_LSB			0x1a
#define EULER_H_REGISTER_MSB			0x1b
#define CALIBRATION_STATUS				0x35
#define	SYS_TRIGGER								0x3f

#define BNO055_CHIP_ID						0xa0
#define BNO055_ACC_ID							0xfb
#define BNO055_MAG_ID							0x32
#define BNO055_GYRO_ID						0x0f

#define UNIT_ACC_MSS							0x00
#define UNIT_GYR_DPS							0x00
#define UNIT_EULER_DEG						0x00
#define UNIT_TEMP_C								0x00
#define UNIT_ORI_WIN							0x00

#define NDOF_FUSION_MODE 					0x0c
#define IMU_FUSION_MODE						0x08

#define BNO055_I2C_ADDRESS 				0x28

#define PIPAS_SCL_PIN							3
#define PIPAS_SDA_PIN							2

#define BNO055_TEMP_ADDR 					0X34

void init_twi();
void BNO055_init();
void BNO055_select_register_map_page(uint8_t page);
void BNO055_set_power_mode();
void BNO055_select_units();
void BNO055_select_ndof_fusion();
void BNO055_select_internal_oscillator();
bool BNO055_check_device_id();
uint8_t BNO055_calibration_status();
void BNO055_read_euler_angles(double*, double*, double*);
void BNO055_read_accel(double*, double*, double*);
