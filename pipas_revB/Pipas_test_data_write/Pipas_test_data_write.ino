#include "pipas.h"

#include <SerialFlash.h>


const char* filename = "test.txt";


void setup() {
  Serial.begin(9600);

  pinMode(STATUS_L, OUTPUT);
  pinMode(STATUS_R, OUTPUT);
  pinMode(TOUCH, INPUT_PULLUP);

  while (!Serial);

  SerialFlash.begin(14);
  
  while (!SerialFlash.ready());

  SerialFlash.create(filename, 256);

  unsigned int n;
  n = 8;
  char buf[256];
  buf[0] = 's';


  SerialFlashFile flashFileOut = SerialFlash.open(filename);
  Serial.println("w");
  flashFileOut.write(buf, n);
  Serial.println("ww");
  flashFileOut.close();
  if (SerialFlash.exists(filename)) {
    SerialFlashFile fileopen;
    uint32_t sz = fileopen.size();
    uint32_t pos = fileopen.getFlashAddress();
    Serial.println(filename);
    Serial.println(sz);
    Serial.println(pos);
  }
  if (SerialFlash.exists(filename)) {
    Serial.println(filename);
    digitalWrite(29, HIGH);
    delay(300);
    digitalWrite(29, LOW);

  }


}

void loop() {
}


void BlinkLed_(int n, int flashing) {
  for (int i = 0; i < n; i++) {
    digitalWrite(STATUS_L, HIGH);
    digitalWrite(STATUS_R, HIGH);
    delay(flashing);
    digitalWrite(STATUS_L, LOW);
    digitalWrite(STATUS_R, LOW);
    delay(flashing);
  }
  delay(10);
}
