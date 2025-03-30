#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE, CSN

const byte address[6] = "00001";
char buffer[32];

void setup() {
    Serial.begin(115200);
    radio.begin();
    radio.setPALevel(RF24_PA_HIGH);
    radio.setDataRate(RF24_250KBPS);
    radio.setRetries(5, 15); // Add retries
    radio.openWritingPipe(address);
    radio.openReadingPipe(1, address);
    radio.startListening();
}

void loop() {
    if (Serial.available()) {
        int len = Serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
        buffer[len] = '\0';
        radio.stopListening();
        bool success = radio.write(buffer, len + 1);
        delay(5);
        radio.startListening();
        if (!success) {
            Serial.println("[Error] Send failed");
        }
    }
    
    if (radio.available()) {
        radio.read(buffer, sizeof(buffer));
        Serial.println(buffer);
    }
}
