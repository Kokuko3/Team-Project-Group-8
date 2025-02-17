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
            radio.read(&imgSize, sizeof(imgSize));  // Read image size
<<<<<<< HEAD
            Serial.println("START");  // Notify Python
            Serial.write((char*)&imgSize, sizeof(imgSize));  // Send size
            receiving = true;
        }

        radio.read(buffer, sizeof(buffer));  // Read data, but no return value
        int bytesRead = sizeof(buffer);  // Assume full buffer size
=======
            Serial.println("START");  // Ensure the "START" message ends with newline
            Serial.write((char*)&imgSize, sizeof(imgSize));  // Send image size as raw bytes
            receiving = true;
        }

        radio.read(buffer, sizeof(buffer));
        int bytesRead = sizeof(buffer);
>>>>>>> 579c026e966851a5a99b3607aec8629f8cf09ee1

        Serial.write(buffer, bytesRead);  // Send chunk over Serial

        imgSize -= bytesRead;
        if (imgSize <= 0) {
<<<<<<< HEAD
            Serial.println("END");  // Notify Python
=======
            Serial.println("END");  // Ensure the "END" message ends with newline
>>>>>>> 579c026e966851a5a99b3607aec8629f8cf09ee1
            receiving = false;
        }
    }
}


