/*
    Reference: 
        * https://create.arduino.cc/projecthub/shakataganai/hid-prox-rfid-to-arduino-bd9b8a
        * https://naylampmechatronics.com/blog/18_Tutorial-M%C3%B3dulo-GPS-con-Arduino.html

*/

#include <WiegandNG.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

char dato = ' ';
TinyGPS gps;
SoftwareSerial ss(7, 6);
int wait=5;
int act=0;

WiegandNG wg;

void PrintBinary(WiegandNG & tempwg) {
    volatile unsigned char * buffer=tempwg.getRawData();
    unsigned int bufferSize = tempwg.getBufferSize();
    unsigned int countedBits = tempwg.getBitCounted();

    unsigned int countedBytes = (countedBits / 8);
    if ((countedBits % 8) > 0) countedBytes++;

    String data = "";

    for (unsigned int i = bufferSize - countedBytes; i < bufferSize; i++) {
        unsigned char bufByte = buffer[i];
        for (int x = 0; x < 8; x++) {
            if ((((bufferSize - i) * 8) - x) <= countedBits) {
                if ((bufByte & 0x80)) {
                    data += "1";
                }
                else {
                    data += "0";
                }
            }
            bufByte <<= 1;
        }
    }
    Serial.print(data);
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    ss.begin(9600);

    unsigned int pinD0 = 2;
    unsigned int pinD1 = 3;
    unsigned int wiegandbits = 48;
    unsigned int packetGap = 15;      // 25 ms between packet

    if (!wg.begin(pinD0, pinD1, wiegandbits, packetGap)) {
        //Serial.println("Out of memory!");
    }
}

void loop() {
    if (wg.available()) {
        wg.pause();             // pause Wiegand pin interrupts
        Serial.print("{");
        Serial.print(wg.getBitCounted()); // display the number of bits counted
        Serial.print(",");
        PrintBinary(wg);
        Serial.println("}");
        wg.clear();             // compulsory to call clear() to enable interrupts for subsequent data
    } else {
        bool newData = false;
        unsigned long chars;
        unsigned short sentences, failed;
        // For one second we parse GPS data and report some key values
        for (unsigned long start = millis(); millis() - start < 1000;)
        {
            while (ss.available()) {
                char c = ss.read();
                if (gps.encode(c)) // Did a new valid sentence come in?
                    newData = true;
            }
        }
        if (newData) {
          if(act>=wait){
            float flat, flon;
            unsigned long age;
            gps.f_get_position(& flat, & flon, & age);
            Serial.print("@");
            Serial.print(flat == TinyGPS:: GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
            Serial.print(",");
            Serial.print(flon == TinyGPS:: GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
            Serial.println("@");
            act=0;
          }else{
            act += 1;
          }
        }
        gps.stats(& chars, & sentences, & failed);
    }
}
