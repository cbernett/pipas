#include <SerialFlash.h>
#include <SPI.h>
#include <BLEPeripheral.h>
#include "BLESerial.h"

#include "pipas.h"

BLESerial bleSerial;

const int FlashChipSelect = 14;

SerialFlashFile file;

const unsigned long testIncrement = 4096;

void setup() {
  // custom services and characteristics can be added as well
  bleSerial.setLocalName("Pipas");

  Serial.begin(9600);
  bleSerial.begin();

  while (!Serial) ;
  delay(100);
}

void loop() {
  bleSerial.poll();

  bleSerial.write("test, hello pipas");
}

void forward() {
  if (bleSerial && Serial) {
    int byte;
    while ((byte = bleSerial.read()) > 0) Serial.write((char)byte);
    while ((byte = Serial.read()) > 0) bleSerial.write((char)byte);
  }
}

// echo all received data back
void loopback() {
  if (bleSerial) {
    int byte;
    while ((byte = bleSerial.read()) > 0) bleSerial.write(byte);
  }
}
