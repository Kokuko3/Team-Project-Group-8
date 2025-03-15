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

    memset(buffer,0x00,sizeof(buffer));
}

void loop() {
     if (radio.available()) {
        radio.read(buffer, sizeof(buffer));
        //Serial.println(buffer); // This appends a new line character to the output of each concatination of the data, which inhibits parsing.
        Serial.write(&buffer[1],buffer[0]); 
    }
 
    if (Serial.available()) {
        //int len = Serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
        int len = Serial.readBytes(&buffer[1],sizeof(buffer)-1);
        buffer[0] = len;
        if(!len) return; // why can't you continue in void loop grrrrr

        // If data was read, transmit.
        radio.stopListening();
        bool success = radio.write(buffer, sizeof(buffer));
        delay(5);
        radio.startListening();
        if (!success) {
            Serial.println("[Error] Send failed");
        }
    }
    

}
