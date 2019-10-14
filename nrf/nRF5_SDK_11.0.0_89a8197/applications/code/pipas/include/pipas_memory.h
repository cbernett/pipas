#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_drv_spi.h"
#include "app_util_platform.h"

#define PIPAS_MISO_PIN	 13
#define PIPAS_SS_PIN 		 14
#define PIPAS_SCK_PIN		 15
#define PIPAS_MOSI_PIN   16

#define CMD_PAGEPROG     0x02
#define CMD_READDATA     0x03
#define CMD_WRITEDISABLE 0x04
#define CMD_READSTAT1    0x05
#define CMD_WRITEENABLE  0x06
#define CMD_SECTORERASE  0x20
#define CMD_CHIPERASE    0x60
#define CMD_ID           0x90

#define STAT_BUSY        0x01
#define STAT_WRTEN       0x02

#define LAST_PAGE				 0xAB

#define PAGE_OFFSET 4

#define SPI_INSTANCE  1 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
volatile bool spi_xfer_done;

typedef struct memory_meta_data {
	uint16_t first_page;
	uint16_t num_pages;
	uint16_t current_page;
} memory_meta_data_t;

typedef struct memory_instance {
	memory_meta_data_t* memory_meta_data;
	nrf_drv_spi_config_t spi_config;
} memory_instance_t;

typedef struct page_header {
	uint8_t last_sample;
	uint8_t sample_size;
} page_header_t;

void init_memory();
void spi_event_handler(nrf_drv_spi_evt_t const * p_event);
void flash_write_enable();
void flash_write_disable();
void flash_erase();
void flash_write_page(uint32_t addr, uint8_t* data, uint16_t data_size);
void flash_read_page(uint32_t addr, uint8_t* data, uint16_t data_size);
void start_log();
void end_log();
void log_sample(char*);
void write_sample_buffer();
void log_playback_prepare();
bool sample_available();
char* next_sample();

uint8_t sample_buffer[256];
uint8_t bytes_written;
page_header_t header;
uint32_t current_page = 0;

void spi_event_handler(nrf_drv_spi_evt_t const * p_event)
{
	spi_xfer_done = true;
}

void init_memory()
{
	//m_i = (memory_instance_t*)malloc(sizeof(memory_instance_t));
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG(SPI_INSTANCE);
	spi_config.miso_pin = PIPAS_MISO_PIN;
	spi_config.ss_pin = PIPAS_SS_PIN;
	spi_config.sck_pin = PIPAS_SCK_PIN;
	spi_config.mosi_pin = PIPAS_MOSI_PIN;
	spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
	APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler));
}

void flash_erase()
{
	flash_write_enable();
	uint8_t cmd = CMD_CHIPERASE;
	spi_xfer_done = false;
	nrf_gpio_pins_clear(14);
	nrf_drv_spi_transfer(&spi, &cmd, 1, NULL, 0);
	nrf_gpio_pins_set(14);
	while(spi_xfer_done == false)
	{
     __SEV();
     __WFE();
     __WFE();
  }
	flash_write_disable();
}

void flash_write_enable()
{
	uint8_t cmd = CMD_WRITEENABLE;
	spi_xfer_done = false;
	nrf_gpio_pins_clear(14);
	nrf_drv_spi_transfer(&spi, &cmd, 1, NULL, 0);
	nrf_gpio_pins_set(14);
	while(spi_xfer_done == false)
	{
     __SEV();
     __WFE();
     __WFE();
  }
}

void flash_write_disable()
{
	uint8_t cmd = CMD_WRITEDISABLE;
	spi_xfer_done = false;
	nrf_gpio_pins_clear(14);
	nrf_drv_spi_transfer(&spi, &cmd, 1, NULL, 0);
	nrf_gpio_pins_set(14);
	while(spi_xfer_done == false)
	{
     __SEV();
     __WFE();
     __WFE();
  }
}

void flash_write_page(uint32_t addr, uint8_t* data, uint16_t data_size)
{
	uint32_t page = (addr + PAGE_OFFSET) * 256;
	flash_write_enable();
	uint8_t data_buf[260];
	data_buf[0] = CMD_PAGEPROG;
	data_buf[1] = (page >> 16);
	data_buf[2] = (page >> 8);
	data_buf[3] = 0;
	memcpy(data_buf + 4, data, data_size);
	spi_xfer_done = false;
	nrf_gpio_pins_clear(14);
	nrf_gpio_pins_set(14);
	nrf_drv_spi_transfer(&spi, data_buf, 260, NULL, 0);
	while(spi_xfer_done == false)
	{
     __SEV();
     __WFE();
     __WFE();
  }
	flash_write_disable();
}

void start_log()
{
	flash_erase();
	current_page = 0;
	bytes_written = 2;
	memset(sample_buffer, 0xff, 256);
	while(current_page < PAGE_OFFSET){
		flash_write_page(current_page, sample_buffer, 256);
		current_page = current_page + 1;
	}
}

void end_log()
{
	sample_buffer[0] = bytes_written;
	sample_buffer[1] = LAST_PAGE;
	write_sample_buffer();
}

void log_sample(char* sample)
{
	//header_t* header = malloc(sizeof(header_t));
	uint8_t data_buffer[256];
	uint8_t len = strlen(sample);
	if(bytes_written + len > 256){
		sample_buffer[0] = bytes_written;
		sample_buffer[1] = 0x00;
		//sample_buffer[0] = current_page - PAGE_OFFSET;
		//sample_buffer[1] = current_page - PAGE_OFFSET;
		write_sample_buffer();
	}
	memcpy(sample_buffer + bytes_written, sample, len);
	bytes_written += len;
	//write_sample_buffer();
}

void write_sample_buffer()
{
		flash_write_page(current_page, sample_buffer, 256);
		if(current_page < 4096){
			current_page = current_page + 1;
		}
		bytes_written = 0;
}

void flash_read_page(uint32_t addr, uint8_t* data, uint16_t data_size)
{
	uint32_t page = (addr + PAGE_OFFSET) * 256;
	uint8_t cmd_buf[260];
	uint8_t out_buf[260];
	memset(cmd_buf, 0, 260);
	memset(out_buf, 0, 260);
	cmd_buf[0] = CMD_READDATA;
	cmd_buf[1] = (page >> 16);
	cmd_buf[2] = (page >> 8);
	cmd_buf[3] = 0;
	nrf_drv_spi_transfer(&spi, cmd_buf, 260, out_buf, 260);
	while(spi_xfer_done == false)
	{
     __SEV();
     __WFE();
     __WFE();
  }
	memcpy(data, out_buf + 4, 256);
}

void log_playback_prepare()
{
}

bool sample_available()
{
	return false;
}

char* next_sample()
{
}