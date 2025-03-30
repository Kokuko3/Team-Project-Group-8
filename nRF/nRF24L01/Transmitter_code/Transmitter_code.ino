#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE, CSN pins

const byte address[6] = "00001";
char buffer[32];

void setup() {
    Serial.begin(115200);
    radio.begin();
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_HIGH);
    radio.startListening();
}

void sendChunk(char *data, int len) {
    radio.write(data, len);
}

void loop() {
    if (Serial.available() > 0) {
        radio.stopListening();
        uint32_t imgSize;
        Serial.readBytes((char*)&imgSize, 4);
        Serial.println("Receiving image...");
        for (uint32_t i = 0; i < imgSize; i += 32) {
            int bytesRead = Serial.readBytes(buffer, 32);
            sendChunk(buffer, bytesRead);
        }
        Serial.println("Image transmitted!");
        radio.startListening();
    } 
    else {
        if (radio.available()) {
            radio.read(buffer, sizeof(buffer));
            Serial.write(buffer);
        }
    }
}

