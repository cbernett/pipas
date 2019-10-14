#include <stdlib.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "pipas_imu.h"
#include "pipas_memory.h"
#include "timer.h"
#include "utils.h"
#include "ble_nus.h"
#include "nrf_gpio.h"

#define PIPAS_BUTTON_PIN 9
#define PIPAS_LED_R 20
#define PIPAS_LED_L 29

#define NUM_READINGS 2

void pipas_init(ble_nus_t*);
void pipas_run();
void button_condition();
void sample_imu();
void log_data();
void start_recording();
void stop_recording();
void stream_recording();
void ble_uart_send(uint8_t*, uint8_t);

volatile bool button_active = false;
volatile bool button_status = false;
uint32_t button_timer = 0;
uint32_t long_press_time = 1000;
uint32_t last_send_time = 0;
bool long_press_active = false;
bool bt_send = false;
bool write_flash = false;
uint64_t last_blink = 0;
static ble_nus_t* m_nus_ptr;

typedef struct imu_sample 
{
	double accel_x;
	double accel_y;
	double accel_z;
	double euler_yaw;
	double euler_roll;
	double euler_pitch;
} imu_sample_t;

double x_readings[NUM_READINGS];
int x_read_index = 0;
double x_total = 0.0;
double x_average = 0.0;
	
double y_readings[NUM_READINGS];      
int y_read_index = 0;              
double y_total = 0;                
double y_average = 0; 

double z_readings[NUM_READINGS];      
int z_read_index = 0;              
double z_total = 0;                
double z_average = 0; 

void pipas_init(ble_nus_t* _m_nus)
{
	nrf_gpio_cfg_input(PIPAS_BUTTON_PIN, BUTTON_PULL);
	nrf_gpio_cfg_output(PIPAS_LED_R);
	nrf_gpio_cfg_output(PIPAS_LED_L);
	nrf_gpio_pin_clear(PIPAS_LED_L);
	nrf_gpio_pin_clear(PIPAS_LED_R);
	
	init_memory();
	init_imu();
	
	nrf_gpio_pin_set(PIPAS_LED_L);
	nrf_gpio_pin_set(PIPAS_LED_R);
	nrf_delay_ms(100);
	nrf_gpio_pin_clear(PIPAS_LED_L);
	nrf_gpio_pin_clear(PIPAS_LED_R);
	
	timer_start();
	m_nus_ptr = _m_nus;
}

void button_condition()
{
	if(button_status == false)
	{
		if(button_active == false)
		{
			button_active = true;
			button_timer = millis();
		}
	
		if((millis() - button_timer > long_press_time) && (long_press_active == false))
		{
			long_press_active = true;
			bt_send = true;
		}
	} else {
		if(button_active == true)
		{
			if(long_press_active == true)
			{
				long_press_active = false;
			} else {
				write_flash = !write_flash;
				nrf_gpio_pin_set(PIPAS_LED_L);
				nrf_gpio_pin_set(PIPAS_LED_R);
				if(write_flash)
				{
					//init_imu();
					//nrf_delay_ms(10);
					start_log();
				} else {
					end_log();
				}
				nrf_gpio_pin_clear(PIPAS_LED_L);
				nrf_gpio_pin_clear(PIPAS_LED_R);
			}
			
			button_active = false;
		}
	}
	nrf_delay_ms(15);
}

void pipas_run()
{
	button_status = nrf_gpio_pin_read(PIPAS_BUTTON_PIN);
	button_condition();
	
	if(bt_send) 
	{
		bt_send = false;
		nrf_gpio_pin_set(PIPAS_LED_L);
		nrf_gpio_pin_set(PIPAS_LED_R);
		nrf_delay_ms(100);
		nrf_gpio_pin_clear(PIPAS_LED_L);
		nrf_gpio_pin_clear(PIPAS_LED_R);
		nrf_delay_ms(100);
		nrf_gpio_pin_set(PIPAS_LED_L);
		nrf_gpio_pin_set(PIPAS_LED_R);
		nrf_delay_ms(100);
		nrf_gpio_pin_clear(PIPAS_LED_L);
		nrf_gpio_pin_clear(PIPAS_LED_R);
		
		log_playback_prepare();
		uint8_t c = '>';
		ble_uart_send(&c, 1);
		stream_recording();
		c = '<';
		ble_uart_send(&c, 1);
		nrf_gpio_pin_set(PIPAS_LED_L);
		nrf_gpio_pin_set(PIPAS_LED_R);
		nrf_delay_ms(100);
		nrf_gpio_pin_clear(PIPAS_LED_L);
		nrf_gpio_pin_clear(PIPAS_LED_R);
	}
	
	if(write_flash)
	{
		if(millis() - last_send_time > 100)
		{
			nrf_gpio_pin_set(PIPAS_LED_L);
			nrf_gpio_pin_set(PIPAS_LED_R);
			log_data();
			//nrf_delay_ms(50);
			nrf_gpio_pin_clear(PIPAS_LED_L);	
			nrf_gpio_pin_clear(PIPAS_LED_R);
		}
	}
}

void sample_imu(imu_sample_t* s)
{
	imu_get_accel(&s->accel_x, &s->accel_y, &s->accel_z);
	imu_get_euler_angle(&s->euler_yaw, &s->euler_roll, &s->euler_pitch);
}

void log_data()
{
	imu_sample_t* i_s = malloc(sizeof(imu_sample_t));
	sample_imu(i_s);
	x_total = x_total - x_readings[x_read_index];
	x_readings[x_read_index] = i_s->accel_x;
	x_total = x_total + x_readings[x_read_index];
	x_read_index = x_read_index + 1;
	
	if(x_read_index >= NUM_READINGS)
	{
		x_read_index = 0;
	}
	
	y_average = y_total / NUM_READINGS;
	
	y_total = y_total - y_readings[y_read_index];
	y_readings[y_read_index] = i_s->accel_y;
	y_total = y_total + y_readings[y_read_index];
	y_read_index = y_read_index + 1;
	
	if(y_read_index >= NUM_READINGS)
	{
		y_read_index = 0;
	}
	
	y_average = y_total / NUM_READINGS;
	
	z_total = z_total - z_readings[z_read_index];
	z_readings[z_read_index] = i_s->accel_z;
	z_total = z_total + z_readings[z_read_index];
	z_read_index = z_read_index + 1;
	
	if(z_read_index >= NUM_READINGS)
	{
		z_read_index = 0;
	}
	
	z_average = z_total / NUM_READINGS;
	
	char data_string[256];
	sprintf(data_string, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", millis() - last_send_time, i_s->euler_yaw, i_s->euler_roll, i_s->euler_pitch, x_average, y_average, z_average);
	log_sample(data_string);
	last_send_time = millis();
}

void stream_recording()
{
	uint32_t current_page = PAGE_OFFSET;
	uint8_t buffer[260];
	memset(buffer, 0, 260);
	while(current_page < 4096){
		flash_read_page(current_page, buffer, 260);
		uint8_t bytes_in_buffer = buffer[0];
		uint8_t bytes_written = 0;
		char s[100];
		sprintf(s, "%d:%d\n", current_page, bytes_in_buffer);
		while(bytes_in_buffer - bytes_written > 20) 
		{
			ble_uart_send((uint8_t*)&buffer[2 + bytes_written], 20);
			bytes_written += 20;
			nrf_delay_ms(20);
		}
		ble_uart_send((uint8_t*)&buffer[2 + bytes_written], bytes_in_buffer - bytes_written);
		nrf_delay_ms(20);
		if(buffer[1] != LAST_PAGE){
			current_page = current_page + 1;
		} else {
			break;
		}
		nrf_delay_ms(30);
	}
}

void ble_uart_send(uint8_t* buffer, uint8_t size)
{
	uint8_t bytes_to_send = size;
	uint8_t offset = 0;
	while(bytes_to_send > 20)
	{
		ble_nus_string_send(m_nus_ptr, buffer + offset, 20);
		bytes_to_send -= 20;
		offset += 20;
	}
	ble_nus_string_send(m_nus_ptr, buffer + offset, bytes_to_send);
}