#pragma once
#include "Wire.h"

#define internalWire Wire
// TwoWire externalWire(&sercom1, PIN_PA16, PIN_PA17);
#define externalWire Wire1

#define I2C_ADDR_BASE 0x20

void I2C_RxHandler(int numBytes);
void I2C_TxHandler();

uint8_t getAddress(uint8_t base_addr, uint32_t addr_pins[])
{
    auto addr = base_addr;
    for (auto i = 0; i < 2; i++)
    {
        pinMode(addr_pins[i], INPUT);
        auto val = digitalRead(addr_pins[i]);
        if (val == LOW)
        {
            addr |= (1 << i);
        }
    }
    return addr;
}

void initExternalWire()
{
    externalWire.begin(ext_addr);
    externalWire.onReceive(I2C_RxHandler);
    externalWire.onRequest(I2C_TxHandler);
}

void broadcast(uint8_t reg, uint8_t data[], uint8_t length)
{
    externalWire.end();
    externalWire.begin();
    auto addr = I2C_ADDR_BASE;
    for (auto i = 0; i < 4; i++)
    {
        if (addr == ext_addr)
        {
            addr++;
            continue;
        }
        SerialUSB.print("[i2c] to 0x");
        externalWire.beginTransmission(addr);
        SerialUSB.print(addr, HEX);
        SerialUSB.print(" Reg: 0x");
        externalWire.write(reg);
        SerialUSB.print(reg, HEX);
        SerialUSB.print(" Data:");
        for (auto j = 0; j < length; j++)
        {
            SerialUSB.print(" 0x");
            SerialUSB.print(data[j], HEX);
            externalWire.write(data[j]);
        }
        externalWire.endTransmission();
        SerialUSB.println();
        addr++;
    }

    externalWire.end();
    initExternalWire();
}

void scan(TwoWire WIRE)
{
    byte error, address;
    int nDevices;

    SerialUSB.println("Scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        WIRE.beginTransmission(address);
        error = WIRE.endTransmission();

        if (error == 0)
        {
            SerialUSB.print("I2C device found at address 0x");
            if (address < 16)
                SerialUSB.print("0");
            SerialUSB.print(address, HEX);
            SerialUSB.println("  !");

            WIRE.requestFrom(address, 4);
            SerialUSB.println("Data: ");
            SerialUSB.println(WIRE.read(), BIN);
            SerialUSB.println(WIRE.read(), BIN);
            SerialUSB.println(WIRE.read(), BIN);
            SerialUSB.println(WIRE.read(), BIN);

            nDevices++;
        }
        else if (error == 4)
        {
            SerialUSB.print("Unknown error at address 0x");
            if (address < 16)
                SerialUSB.print("0");
            SerialUSB.println(address, HEX);
        }
    }
    if (nDevices == 0)
        SerialUSB.println("No I2C devices found\n");
    else
        SerialUSB.println("done\n");

    delay(5000); // wait 5 seconds for next scan
}