#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE, CSN pins
const byte address[6] = "00001";  // Same as transmitter

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
            radio.read(&imgSize, sizeof(imgSize));  // Read image size
            Serial.write("START\n");  // Notify Python
            Serial.write((char*)&imgSize, sizeof(imgSize));  // Send size
            receiving = true;
        }

        int bytesRead = radio.read(buffer, sizeof(buffer));
        Serial.write(buffer, bytesRead);  // Send chunk over Serial

        imgSize -= bytesRead;
        if (imgSize <= 0) {
            Serial.write("END\n");  // Notify Python
            receiving = false;
        }
    }
}
