#pragma once
#include "i2c_bus.hpp"

#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PCA9685_I2C_ADDRESS, internalWire);

u_int16_t shuffleTime = 8000;
u_int16_t old_ledState = 0;
u_int16_t ledState = 0;
u_int16_t alwaysOn = 1;
uint16_t maxBrightness = 4096;
uint16_t ledBrightness[16] =
    {2048, maxBrightness, maxBrightness, maxBrightness,
     maxBrightness, maxBrightness, maxBrightness, maxBrightness,
     maxBrightness, maxBrightness, maxBrightness, maxBrightness,
     maxBrightness, maxBrightness, maxBrightness, maxBrightness};
bool enabled = true;

void _ledOn(uint8_t pin, uint16_t brightness = 4096, uint16_t step = 1)
{
    for (uint16_t i = 0; i < brightness; i += step)
    {
        pwm.setPin(pin, i, true);
    }
}

uint8_t _pin = 0;
uint16_t _brightness = 4096;
uint16_t _step = 8;

void _ledOff(uint8_t pin, uint16_t brightness = 4096, uint16_t step = 1)
{
    for (uint16_t i = 0; i < brightness; i += step)
    {
        pwm.setPin(pin, i);
    }
    pwm.setPin(pin, brightness);
}

void ledOn(uint8_t pin, uint16_t brightness = 4096, uint16_t step = 1)
{
    _pin = pin;
    _brightness = brightness;
    _step = step;
    Task *t = new Task(
        0, 1, []()
        { _ledOn(_pin, _brightness, _step); },
        &runner);
    t->enable();
}

void ledOff(uint8_t pin, uint16_t brightness = 4096, uint16_t step = 1)
{
    _pin = pin;
    _brightness = brightness;
    _step = step;
    Task *t = new Task(
        0, 1, []()
        { _ledOff(_pin, _brightness, _step); },
        &runner);
    t->enable();
}

u_int16_t getDiffMaskOn()
{
    u_int16_t ret = 0;
    auto old_mask = old_ledState;
    auto mask = ledState;
    uint8_t p = 0;
    while (p < 16)
    {
        auto n = mask & 1;
        auto o = old_mask & 1;
        if (n && !o)
        {
            ret |= 1 << p;
        }

        mask >>= 1;
        old_mask >>= 1;
        p++;
    }
    return ret;
}

u_int16_t getDiffMaskOff()
{
    u_int16_t ret = 0;
    auto old_mask = old_ledState;
    auto mask = ledState;
    uint8_t p = 0;
    while (p < 16)
    {
        auto n = mask & 1;
        auto o = old_mask & 1;
        if (!n && o)
        {
            ret |= 1 << p;
        }

        mask >>= 1;
        old_mask >>= 1;
        p++;
    }
    return ret;
}

uint16_t _brightness_on = 0;
void ledOnByMask()
{
    Task *t = new Task(
        1, maxBrightness / _step, []()
        { 
        if(_brightness_on >= maxBrightness){
            _brightness_on = 0;
        }
        auto mask = getDiffMaskOn();
        uint8_t p = 0;
        while (p < 16)
        {
            if(mask & 1 && !(_brightness_on > ledBrightness[p])){
                pwm.setPin(p, _brightness_on, true);
            }
            mask >>= 1;
            p++;
        }
        _brightness_on+=_step; },
        &runner);
    t->enable();
}

uint16_t _brightness_off = maxBrightness;
void ledOffByMask()
{
    Task *t = new Task(
        1, maxBrightness / _step, []()
        { 
        if(_brightness_off <= 0){
            _brightness_off = maxBrightness;
        }
        auto mask = getDiffMaskOff();
        uint8_t p = 0;
        while (p < 16)
        {
            if(mask & 1 && !(_brightness_on > ledBrightness[p])){
                pwm.setPin(p, _brightness_off, true);
            }
            mask >>= 1;
            p++;
        }
        _brightness_off-=_step; },
        &runner);
    t->enable();
}

void applyState()
{
    // SerialUSB.println(old_ledState, BIN);
    // auto mask_on = getDiffMaskOn();
    // auto mask_off = getDiffMaskOff();
    // SerialUSB.print("Mask on: ");
    // SerialUSB.println(mask_on, BIN);
    // SerialUSB.print("Mask off: ");
    // SerialUSB.println(mask_off, BIN);
    // SerialUSB.println(ledState, BIN);
    ledOnByMask();
    ledOffByMask();
}

void shuffle()
{
    old_ledState = ledState;
    ledState = random(0, 65535);
    ledState |= alwaysOn;
    if (!enabled)
    {
        ledState = 0;
    }
    applyState();
}

void clearLeds()
{
    ledState = 0;
    old_ledState = ledState;
    for (uint8_t i = 0; i < 16; i++)
    {
        pwm.setPin(i, 0, true);
    }
}

void offAllLeds()
{
    ledState = 0;
    old_ledState = ledState;
    applyState();
}

void onAllLeds()
{
    ledState = 0xffff;
    old_ledState = 0;
    applyState();
}

Task shuffleTask(shuffleTime, TASK_FOREVER, &shuffle);

bool initLedDriver()
{
    if (pwm.begin())
    {
        pwm.setPWMFreq(1000);
        SerialUSB.println("LED Driver inited.");
    }
    else
    {
        SerialUSB.println("LED Driver init failed.");
        return false;
    }
    runner.addTask(shuffleTask);
    shuffleTask.enable();
    internalWire.setClock(400000);
    dumpEEPROM();

    // byte b[4] = {0x1, 0x1, 0x1, 0x1};
    // for (int i = 0; i < 4; i++)
    // {
    //     b[i] = eeprom.readByte(ALWAYS_ADDR + i);
    //     printBin(b[i]);
    //     SerialUSB.print(" ");
    // }
    // SerialUSB.println("");
    // alwaysOn = b[0] << 8 | b[1];

    byte b[2];
    eeprom.readBlock(ALWAYS_ADDR, b, 2);
    byte a = b[0];
    eeprom.readBlock(ALWAYS_ADDR + 8, b, 2);
    byte a1 = b[0];
    alwaysOn = a << 8 | a1;
    return true;
}

void setEnabled(bool state)
{
    enabled = state;

    SerialUSB.print("[leds] enabled: ");
    SerialUSB.println(enabled);
    if (enabled)
    {
        shuffle();
    }
    else
    {
        offAllLeds();
        clearLeds();
    }
}