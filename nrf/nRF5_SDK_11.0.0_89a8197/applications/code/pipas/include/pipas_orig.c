#include "pipas.h"

/*------- Variables: Button -------*/
int b_button_active = 0;
int b_long_press_active = 0;

long button_timer = 0;
long long_press_time = 1000;

int button = 9;
int button_read;

int b_write_flash = 0;
int b_bt_send = 0;
/*---------------------------------*/

/*------- Variables: SD -------*/
uint8_t sd_card_present = 0;
char* log_filename;
char* log_file_buffer;
char* backup_filename;
char* backup_file_buffer;
char* log_file;
char* backup_file;
/*-----------------------------*/

unsigned long last_send_time = 0;
unsigned long last_sample_time = 0;
#define SAMPLE_DELAY 100

/*------- Variables: smoothing -------*/
const int num_readings = 2;

float x_readings[num_readings];      // the readings from the input
int x_readIndex = 0;              // the index of the current reading
float x_total = 0;                  // the running total
float x_average = 0;                // the average

float y_readings[num_readings];      // the readings from the input
int y_read_index = 0;              // the index of the current reading
float y_total = 0;                  // the running total
float y_average = 0;                // the average

float z_readings[num_readings];      // the readings from the input
int x_read_index = 0;              // the index of the current reading
float z_total = 0;                  // the running total
float z_average = 0;                // the average

uint8_t enable_sd_logging = ENABLE_SD_LOGGING;
uint8_t enable_serial_logging = ENABLE_UART_LOGGING;
uint8_t enable_time_log = ENABLE_TIME_LOG;
uint8_t enableCalculatedValues = ENABLE_CALCULATED_LOG;
uint8_t enableAccel = ENABLE_ACCEL_LOG;
uint8_t enableGyro = ENABLE_GYRO_LOG;
uint8_t enableCompass = ENABLE_MAG_LOG;
uint8_t enableQuat = ENABLE_QUAT_LOG;
uint8_t enableEuler = ENABLE_EULER_LOG;
uint8_t enableHeading = ENABLE_HEADING_LOG;


unsigned short accel_FSR = IMU_ACCEL_FSR;
unsigned short gyro_FSR = IMU_GYRO_FSR;
unsigned short fifo_rate = DMP_SAMPLE_RATE;

uint8_t init_IMU()
{
	return 1;
}

uint8_t init_sd()
{
	return 1;
}

void println(const char* s)
{
	
}

void setup() {
  // put your setup code here, to run once:
      if ( !init_IMU() )
  {
    println("Error connecting to MPU-9250");
    while (1) ;
  }

  if ( init_sd() )
  {
    log_filename = "PIPAS.txt";
  }

  //pinMode(HW_LED_PIN, OUTPUT);
	nrf_gpio_cfg_output(HW_LED_PIN);

  //SerialUSB.begin(SERIAL_BAUD_RATE);
  //Serial1.begin(SERIAL_BAUD_RATE);

  //pinMode(button, INPUT_PULLUP);
	nrf_gpio_cfg_input(button, NRF_GPIO_PIN_PULLUP);

  for (int i = 0; i < num_readings; i++) {
    x_readings[i] = 0;
    y_readings[i] = 0;
    z_readings[i] = 0;
  }

  /*
	if (!SD.exists("/backup")) {
    SD.mkdir("/backup");
    logFile = SD.open("/backup/default.txt", FILE_WRITE);
    logFile.close();
  }
	*/
  
	//digitalWrite(HW_LED_PIN, HIGH);
  nrf_gpio_pin_set(HW_LED_PIN);
	nrf_delay_ms(100);
  nrf_gpio_pin_clear(HW_LED_PIN);

}

/*
void loop() {
  button_read = nrf_gpio_pin_read(button);
  button_condition();

  if (WriteSD && !logFile) {
    SD.remove("PIPAS.txt");
    backupFileName = nextLogFile();
    logFile = SD.open("PIPAS.txt", FILE_WRITE);
    backupFile = SD.open("/backup/" + backupFileName, FILE_WRITE);

    logFile.println("PIPAS v2.1");
    logFile.println("Back-up File: " + String(backupFileName));  logFile.println("");
    logFile.println("Sensor Data");
    logFile.println("data list:");
    logFile.println("time\tyaw_z\tpitch_y\troll_x\tlinA_x\tlinA_y\tlinA_z\t");


    backupFile.println("PIPAS v2.1");
    backupFile.println("Back-up File: " + String(backupFileName)); backupFile.println("");
    backupFile.println("Sensor Data");
    backupFile.println("data list:");
    backupFile.println("time\tyaw_z\tpitch_y\troll_x\tlinA_x\tlinA_y\tlinA_z\t");
    SerialUSB.println("open File");
  }

  if (millis() - lastSampleTime > SAMPLE_DELAY) {
    if ( imu.fifoAvailable() )
    {
      // Use dmpUpdateFifo to update the ax, gx, mx, etc. values
      if ( imu.dmpUpdateFifo() == INV_SUCCESS)
      {
        imu.update(UPDATE_ACCEL);
        imu.computeEulerAngles();

        logData();

        if (logFile && !WriteSD) {
          logFile.close();
          backupFile.close();
          SerialUSB.println("close File");
        }

      }
      lastSampleTime = millis();
    }
  }

  if (BTsend) {
    BTsend = false;
    digitalWrite(HW_LED_PIN, HIGH);
    delay(100);
    digitalWrite(HW_LED_PIN, LOW);
    delay(100);
    digitalWrite(HW_LED_PIN, HIGH);
    delay(100);
    digitalWrite(HW_LED_PIN, LOW);

    logFile = SD.open("PIPAS.txt");
    Serial1.write(">");
    while (logFile.available() > 0) {
      Serial1.write(logFile.read());
    }
    Serial1.write("<");
    logFile.close();

    digitalWrite(HW_LED_PIN, HIGH);
    delay(100);
    digitalWrite(HW_LED_PIN, LOW);

  }
  digitalWrite(HW_LED_PIN, WriteSD);
}

void button_condition() {
  if (button_read == 0) {

    if (b_button_active == 0) {
      b_button_active = true;
      button_timer = millis();
    }

    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
      longPressActive = true;
      BTsend = true;
    }


  } else {
    if (buttonActive == true) {
      if (longPressActive == true) {
        longPressActive = false;
      } else {
        WriteSD = ! WriteSD;
      }

      buttonActive = false;


    }

  }

  delay(15);
}

*/
