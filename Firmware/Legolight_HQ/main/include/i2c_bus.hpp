#pragma once
#include "include/device.hpp"
#include <vector>

void broadcast(uint8_t reg, uint8_t data[], uint8_t length)
{
    auto addr = I2C_ADDR_BASE;
    for (auto i = 0; i < 4; i++)
    {
        Wire.beginTransmission(addr);
        Wire.write(reg);
        for (auto j = 0; j < length; j++)
        {
            Wire.write(data[j]);
        }
        Wire.endTransmission();
        addr++;
    }
}

std::vector<Device> findDevices()
{
    std::vector<Device> found_devices;
    int address;
    int res = -1;
    for (address = I2C_ADDR_BASE; address < I2C_ADDR_BASE + I2C_ADDR_COUNT; address++)
    {
        res = 0;
        auto answer_len = 7;
        res = Wire.requestFrom(address, answer_len);
        byte data[answer_len];
        if (res != 0)
        {
            Wire.readBytes(data, answer_len);

            auto d = Device{
                address: address,
                version: data[0],
                config: data[1],
                led_state0: data[2],
                led_state1: data[3],
                always_on0: data[4],
                always_on1: data[5],
                rgb_enabled: false,
                meta: DeviceConfig()
            };
            found_devices.push_back(d);
        }
    }

    auto d = Device{
        address: 0x20,
        version: 1,
        config: 0b11000000,
        led_state0: 0b11000000,
        led_state1: 0b00000011,
        always_on0: 0b00000000,
        always_on1: 0b00000000,
        rgb_enabled: false,
        meta: DeviceConfig()
    };
    found_devices.push_back(d);

    return found_devices;
}