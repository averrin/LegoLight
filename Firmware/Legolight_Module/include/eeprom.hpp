#include <Wire.h>
#include <I2C_eeprom.h>
#include "Arduino.h"

// it's only 1Kbit!!!
#define EE24LC01MAXBYTES 1024 / 8

// the address of your EEPROM
#define DEVICEADDRESS (0x50)

#define TEST_ADDR 8

// this must start on a page boundary!
#define TEST_PAGE_ADDR 0
// this tests multi-page writes
#define LONG_BUFFER_LEN 64

I2C_eeprom eeprom(DEVICEADDRESS, &internalWire);
#define CONFIG_ADDR 0
#define ALWAYS_ADDR CONFIG_ADDR + 16

void applyConfig();
uint8_t getConfig()
{
    byte b[2];
    eeprom.readBlock(CONFIG_ADDR, b, 2);
    return b[0];
}

uint8_t saveConfig(uint8_t config)
{
    byte b[1] = {config};
    auto r = eeprom.writeBlock(CONFIG_ADDR, b, 1);
    if (r != 0)
    {
        error = true;
        SerialUSB.println("Config writing error");
    }
    return getConfig();
}

void readAndWriteVar()
{
    SerialUSB.println("----------------------------------------------");
    SerialUSB.print("SINGLE BYTE: writing and retreiving EEPROM on I2C at address ");
    SerialUSB.println(DEVICEADDRESS);
    SerialUSB.println("----------------------------------------------");

    char curval = eeprom.readByte(TEST_ADDR);

    SerialUSB.print("last value: ");
    SerialUSB.println((char)curval);

    curval = random(26) + 'A';
    eeprom.writeByte(TEST_ADDR, curval);

    SerialUSB.print("updating to: ");
    SerialUSB.println((char)curval);
    delay(10);

    curval = eeprom.readByte(TEST_ADDR);
    SerialUSB.print("new value: ");
    SerialUSB.println((char)curval);
}

void readAndWritePage(unsigned int pageAddress, int bufferLen)
{
    // always make the maximum size, just don't use all of it.
    byte testBuffer[LONG_BUFFER_LEN + 1];

    // null-terminate for printing!
    testBuffer[bufferLen] = '\0';

    eeprom.readBlock(pageAddress, testBuffer, bufferLen);

    SerialUSB.print("last value:  ");
    SerialUSB.println((char *)testBuffer);

    for (int i = 0; i < bufferLen; i++)
    {
        // use max to init to all AAAA's on first run.
        testBuffer[i] = 'A';
        char c = random(26) + 'A';
        testBuffer[i] = c;
    }

    eeprom.writeBlock(pageAddress, testBuffer, bufferLen);

    SerialUSB.print("updating to: ");
    SerialUSB.println((char *)testBuffer);
    delay(10);

    eeprom.readBlock(pageAddress, testBuffer, bufferLen);
    SerialUSB.print("new value:   ");
    SerialUSB.println((char *)testBuffer);
}

bool initEEPROM()
{
    // eeprom.setExtraWriteCycleTime(6000);
    return eeprom.begin();
}

void dumpEEPROM()
{
    SerialUSB.println("Dumping EEPROM:");
    auto c = 2;
    for (int i = 0; i < EE24LC01MAXBYTES; i += 8)
    {
        byte b[c];
        eeprom.readBlock(i, b, c);
        SerialUSB.print(i);
        SerialUSB.print(": ");
        printBin(b[0]);
        SerialUSB.println();
    }
    SerialUSB.println();
}

void clearEEPROM()
{
    for (int i = 0; i < EE24LC01MAXBYTES; i += 8)
    {
        eeprom.writeByte(i, 0);
    }
}