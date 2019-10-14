#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "BNO055.h"

void init_imu();
void imu_get_accel(double*, double*, double*);
void imu_get_euler_angles(double*, double*, double*);

void init_imu()
{
	BNO055_init();
}

void imu_get_accel(double* x, double* y, double* z)
{
	BNO055_read_accel(x, y, z);
}

void imu_get_euler_angle(double* h, double* r, double* p)
{
	BNO055_read_euler_angles(h, r, p);
}
