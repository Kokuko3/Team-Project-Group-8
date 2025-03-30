#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE, CSN pins
const byte address[6] = "00001";  // Same as receiver

char buffer[32];
uint32_t imgSize;
bool receiving = false;

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    if (!receiving) {
      radio.read(&imgSize, sizeof(imgSize));
      Serial.write("START");
      Serial.write((char*)&imgSize, sizeof(imgSize);
      receiving = true;
    }

    radio.read(buffer, sizeof(buffer)); 
    int bytesRead = sizeof(buffer); 

    Serial.write(buffer, bytesRead);  // Send chunk over Serial

    imgSize -= bytesRead;
    if (imgSize <= 0) {
      Serial.write("END");  // Notify Python
      receiving = false;
    }
  }
}
