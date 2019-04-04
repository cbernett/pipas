#include <SerialFlash.h>

const char* filename = "test01.txt";


void setup() {
  Serial.begin(9600);
  while (!Serial);

  SerialFlash.begin(14);

  while (!SerialFlash.ready());

  SerialFlash.create(filename, 24);

  SerialFlashFile flashFileOut = SerialFlash.open(filename);
  flashFileOut.write('8',8);
//  flashFileOut.write('b');
//  flashFileOut.write('c');
  flashFileOut.close();

  SerialFlashFile flashFileIn = SerialFlash.open(filename);
  while (flashFileIn.available()) {
    Serial.print("peek = 0x");
    Serial.println(flashFileIn.peek(), HEX);

    Serial.print("read = ");
    Serial.println((char)flashFileIn.read());
  }
}

void loop() {
}
