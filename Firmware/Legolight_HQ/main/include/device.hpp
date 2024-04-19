#pragma once

struct Device {
    int address;
    byte version;
    byte config;
    byte led_state0;
    byte led_state1;
    byte always_on0;
    byte always_on1;

    bool rgb_enabled;
    DeviceConfig meta;
};