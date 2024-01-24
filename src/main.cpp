#include <Wire.h>
#include <Arduino.h>
#include "C24AA025UID.hpp"

constexpr unsigned int CONST_SEQ_ADDR = 0x08;

C24AA025UID clDev(&Wire);

char sequenceA[] = "sequence asd ds qwedaxdcxz sadd a sequence asd cxz sadd a sequence asd ds qwedaxdcxz sadd a\0";
char sequenceB[] = "sequence b b b b b  b b b b b b b sequence b b  b b b b b sequence b b b b b  b b b b b b b\0";
char sequenceC[] = "sequence c c c c c  c c c c c c c sequence c c  c c c c c sequence c c c c c  c c c c c c c\0";

void setup()
{
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    delay(2000);
    clDev.init();
}

void loop()
{
    char buffer[2000];

    static int cycle = 0;
    delay(500);
    digitalToggle(13);

    if ((cycle % 10) == 1)
    {
        (void)clDev.read(CONST_SEQ_ADDR, (TUInt8 *)buffer, sizeof(sequenceA));
        Serial.printf("\r\n\r\n reading from address: %s\r\n", buffer);
    }
    else if (cycle == 4)
    {
        Serial.printf("\r\n\r\n writing sequence to address: %s\r\n", sequenceA);
        delay(10);
        clDev.write(CONST_SEQ_ADDR, (TUInt8 *)sequenceA, sizeof(sequenceA));
    }
    else if (cycle == 20)
    {
        Serial.printf("\r\n\r\n writing sequence to address: %s\r\n", sequenceB);
        delay(10);
        clDev.write(CONST_SEQ_ADDR, (TUInt8 *)sequenceA, sizeof(sequenceB));
    }
    else if (cycle == 30)
    {
        Serial.printf("\r\n\r\n writing sequence to address: %s\r\n", sequenceC);
        delay(10);
        clDev.write(CONST_SEQ_ADDR, (TUInt8 *)sequenceA, sizeof(sequenceC));
    }
    cycle++;
}
