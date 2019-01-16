#include <SparkFunMPU9250-DMP.h>
#include <SD.h>
#include "config.h"
#include <FlashStorage.h>

MPU9250_DMP imu;

/*------- Variables: Button -------*/
boolean buttonActive = false;
boolean longPressActive = false;

long buttonTimer = 0;
long longPressTime = 1000;

int button = 9;
int buttonRead;

bool WriteSD = false;
bool BTsend = false;
/*---------------------------------*/

/*------- Variables: SD -------*/
bool sdCardPresent = false;
String logFileName;
String logFileBuffer;
String backupFileName;
String backupFileBuffer;
File logFile;
File backupFile;
/*-----------------------------*/

unsigned long lastSendTime = 0;
unsigned long lastSampleTime = 0;
#define SAMPLE_DELAY 100

/*------- Variables: smoothing -------*/
const int numReadings = 2;

float Xreadings[numReadings];      // the readings from the input
int XreadIndex = 0;              // the index of the current reading
float Xtotal = 0;                  // the running total
float Xaverage = 0;                // the average

float Yreadings[numReadings];      // the readings from the input
int YreadIndex = 0;              // the index of the current reading
float Ytotal = 0;                  // the running total
float Yaverage = 0;                // the average

float Zreadings[numReadings];      // the readings from the input
int ZreadIndex = 0;              // the index of the current reading
float Ztotal = 0;                  // the running total
float Zaverage = 0;                // the average

bool enableSDLogging = ENABLE_SD_LOGGING;
bool enableSerialLogging = ENABLE_UART_LOGGING;
bool enableTimeLog = ENABLE_TIME_LOG;
bool enableCalculatedValues = ENABLE_CALCULATED_LOG;
bool enableAccel = ENABLE_ACCEL_LOG;
bool enableGyro = ENABLE_GYRO_LOG;
bool enableCompass = ENABLE_MAG_LOG;
bool enableQuat = ENABLE_QUAT_LOG;
bool enableEuler = ENABLE_EULER_LOG;
bool enableHeading = ENABLE_HEADING_LOG;

unsigned short accelFSR = IMU_ACCEL_FSR;
unsigned short gyroFSR = IMU_GYRO_FSR;
unsigned short fifoRate = DMP_SAMPLE_RATE;

void setup() {
  // put your setup code here, to run once:
      if ( !initIMU() )
  {
    SerialUSB.println("Error connecting to MPU-9250");
    while (1) ;
  }

  if ( initSD() )
  {
    sdCardPresent = true;
    logFileName = "PIPAS.txt";
  }

  pinMode(HW_LED_PIN, OUTPUT);

  SerialUSB.begin(SERIAL_BAUD_RATE);
  Serial1.begin(SERIAL_BAUD_RATE);

  pinMode(button, INPUT_PULLUP);

  for (int i = 0; i < numReadings; i++) {
    Xreadings[i] = 0;
    Yreadings[i] = 0;
    Zreadings[i] = 0;
  }

  if (!SD.exists("/backup")) {
    SD.mkdir("/backup");
    logFile = SD.open("/backup/default.txt", FILE_WRITE);
    logFile.close();
  }
  digitalWrite(HW_LED_PIN, HIGH);
  delay(100);
  digitalWrite(HW_LED_PIN, LOW);

}

void loop() {
  buttonRead = digitalRead(button);
  buttonCondition();

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

void buttonCondition() {
  if (buttonRead == LOW) {

    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
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

bool initIMU(void)
{
  if (imu.begin() != INV_SUCCESS)
  {
    while (1)
    {
      SerialUSB.println("Unable to communicate with MPU-9250");
      SerialUSB.println("Check connections, and try again.");
      SerialUSB.println();
      delay(5000);
    }
  }

  imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);

  imu.dmpBegin(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL, 10);
  // Configure sensors:
  // Set gyro full-scale range: 250, 500, 1000, or 2000:
  imu.setGyroFSR(gyroFSR);
  // Set accel full-scale range: 2, 4, 8, or 16 g
  imu.setAccelFSR(accelFSR);
  // Set gyro/accel LPF: 5, 10, 20, 42, 98, 188 Hz
  imu.setLPF(IMU_AG_LPF);
  // Set gyro/accel sample rate: between 4-1000Hz
  imu.setSampleRate(IMU_AG_SAMPLE_RATE);
  // Set compass sample rate: between 4-100Hz
  imu.setCompassSampleRate(IMU_COMPASS_SAMPLE_RATE);

  return true; // Return success
}

bool initSD(void)
{
  if ( !SD.begin(SD_CHIP_SELECT_PIN) )
  {
    return false;
  }
  return true;
}

bool sdLogString(String toLog)
{
  // If the log file opened properly, add the string to it.
  if (logFile && backupFile)
  {
    logFile.print(toLog);
    backupFile.print(toLog);

    return true; // Return success
  }

  return false; // Return fail
}


String nextLogFile(void) {
  String filename;
  int logIndex = 0;

  for (int i = 0; i < LOG_FILE_INDEX_MAX; i++)
  {
    // Construct a file with PREFIX[Index].SUFFIX
    filename = String(LOG_FILE_PREFIX);
    filename += String(logIndex);
    filename += ".";
    filename += String(LOG_FILE_SUFFIX);
    // If the file name doesn't exist, return it
    if (!SD.exists("/backup/" + filename))
    {
      return filename;
    }
    // Otherwise increment the index, and try again
    logIndex++;
  }

  return "";
}


void logData(void)
{
  float accelX = imu.calcAccel(imu.ax);
  float accelY = imu.calcAccel(imu.ay);
  float accelZ = imu.calcAccel(imu.az);

  Xtotal = Xtotal - Xreadings[XreadIndex];
  // read from the sensor:
  Xreadings[XreadIndex] = accelX;
  // add the reading to the total:
  Xtotal = Xtotal + Xreadings[XreadIndex];
  // advance to the next position in the array:
  XreadIndex = XreadIndex + 1;

  // if we're at the end of the array...
  if (XreadIndex >= numReadings) {
    // ...wrap around to the beginning:
    XreadIndex = 0;
  }

  // calculate the average:
  Xaverage = Xtotal / numReadings;

  //----------Y DIRECTION----------

  // subtract the last reading:
  Ytotal = Ytotal - Yreadings[YreadIndex];
  // read from the sensor:
  Yreadings[YreadIndex] = accelY;
  // add the reading to the total:
  Ytotal = Ytotal + Yreadings[YreadIndex];
  // advance to the next position in the array:
  YreadIndex = YreadIndex + 1;

  // if we're at the end of the array...
  if (YreadIndex >= numReadings) {
    // ...wrap around to the beginning:
    YreadIndex = 0;
  }

  // calculate the average:
  Yaverage = Ytotal / numReadings;


  //----------Z DIRECTION----------

  // subtract the last reading:
  Ztotal = Ztotal - Zreadings[ZreadIndex];
  // read from the sensor:
  Zreadings[ZreadIndex] = accelZ;
  // add the reading to the total:
  Ztotal = Ztotal + Zreadings[ZreadIndex];
  // advance to the next position in the array:
  ZreadIndex = ZreadIndex + 1;

  // if we're at the end of the array...
  if (ZreadIndex >= numReadings) {
    // ...wrap around to the beginning:
    ZreadIndex = 0;
  }

  // calculate the average:
  Zaverage = Ztotal / numReadings;

  String imuLog = "";

  imuLog += String(millis() - lastSendTime) + ',';
  imuLog += String(imu.yaw) + ',' + String(imu.pitch) + ',' + String(imu.roll) + ',';
  imuLog += String(Xaverage) + ',' + String(Yaverage) + ',' + String(Zaverage);
  imuLog += "\n";
  lastSendTime = millis();

  // If SD card logging is enabled & a card is plugged in
  if ( sdCardPresent && WriteSD )
  {
    if (imuLog.length() + logFileBuffer.length() >= SD_LOG_WRITE_BUFFER_SIZE)
    {
      sdLogString(logFileBuffer); // Log SD buffer
      logFileBuffer = ""; // Clear SD log buffer
    }
    // Add new line to SD log buffer
    logFileBuffer += imuLog;
  }
}

