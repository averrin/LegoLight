#define VERSION 1
#include "Arduino.h"

#include <TaskScheduler.h>
Scheduler runner;
byte config;
byte ext_addr;

void printBin(int var)
{
  for (unsigned int test = 0x80; test; test >>= 1)
  {
    SerialUSB.write(var & test ? '1' : '0');
  }
}

#include "i2c_bus.hpp"

bool error = false;
bool passive = false;
uint8_t counter = 0;
#include "eeprom.hpp"
#include "led_driver.hpp"

#define EXT_PIXELS_COUNT 4
#include "pixels.hpp"

#define SET_STATUS_reg 0x01
#define SET_STATUS_len 3
#define SET_ENABLED_reg 0x02
#define SET_ENABLED_len 1
#define SET_SHUFFLE_reg 0x03
#define SET_SHUFFLE_len 0
#define SET_ALL_LEDS_reg 0x04
#define SET_ALL_LEDS_len 1

#define SET_LEDS_reg 0x05
#define SET_LEDS_len 3

#define SET_CONFIG_reg 0x06
#define SET_CONFIG_len 1

#define SET_ALWAYS_reg 0x08
#define SET_ALWAYS_len 2

#define SET_PASSIVE_reg 0x07
#define SET_PASSIVE_len 1

// #define GET_CONFIG_reg 0x06
// #define GET_CONFIG_len 1

void setPassive(bool p)
{
  if (passive == p)
  {
    return;
  }
  passive = p;
  if (passive)
  {
    shuffleTask.disable();
    setStatusPixel(statusPixel.Color(100, 100, 0));
  }
  else
  {
    shuffleTask.enable();
    // setStatusPixel(statusPixel.Color(0, 100, 0));
  }
  clearStatusPixel(1000);
}

void applyConfig()
{
  if ((config >> 7) & 1)
  {
    startExternalPixels();
  }
  else
  {
    stopExternalPixels();
  }
}

void I2C_TxHandler()
{
  // SerialUSB.println("[i2c]: tx");
  setPassive(true);
  auto c = config;

  if (enabled)
  {
    c = c ^ 1 << 6;
  }

  uint8_t array[7];
  array[0] = VERSION;
  array[1] = c;
  array[2] = ledState & 0xff;
  array[3] = (ledState >> 8);
  array[4] = alwaysOn & 0xff;
  array[5] = (alwaysOn >> 8);
  array[6] = counter;
  externalWire.write(array, 7);
  counter++;
}

void I2C_RxHandler(int numBytes)
{
  if (numBytes == 0)
  {
    return;
  }
  // setStatusPixel(statusPixel.Color(0, 0, 100));
  while (externalWire.available())
  {
    byte c = externalWire.read();
    SerialUSB.print("[i2c]: reg recv: 0x");
    SerialUSB.println(c, HEX);
    switch (c)
    {
    case 0x00:
      break;
    case SET_STATUS_reg:
    {
      byte r = externalWire.read();
      byte g = externalWire.read();
      byte b = externalWire.read();
      setStatusPixel(statusPixel.Color(r, g, b));
      return;
      break;
    }
    case SET_ENABLED_reg:
    {
      byte en = externalWire.read();
      setEnabled(en);
      break;
    }
    case SET_SHUFFLE_reg:
    {
      shuffle();
      break;
    }
    case SET_ALL_LEDS_reg:
    {
      byte en = externalWire.read();
      if (en)
      {
        onAllLeds();
        break;
      }
    }
    case SET_CONFIG_reg:
    {
      byte c = externalWire.read();
      config = c;
      config = saveConfig(config);
      applyConfig();
      break;
    }
    case SET_ALWAYS_reg:
    {
      byte c[2];
      externalWire.readBytes(c, 2);
      eeprom.writeByteVerify(ALWAYS_ADDR, c[1]);
      eeprom.writeByteVerify(ALWAYS_ADDR + 8, c[0]);
      alwaysOn = c[1] << 8 | c[0];
      break;
    }
    case SET_PASSIVE_reg:
    {
      byte p = externalWire.read();
      setPassive(p);
      break;
    }
    default:
      error = true;
      setStatusPixel(statusPixel.Color(100, 0, 0));
      break;
    }
    if (!error)
    {
      clearStatusPixel();
    }
  }
}

#include <EncButton.h>
Button btn1(PIN_PA15);
Button btn2(PIN_PA14);
Button btn3(PIN_PA11);
VirtButton btn12;
VirtButton btn13;
VirtButton btn23;

void buttonHandler()
{
  // SerialUSB.print(".");
  btn1.tick();
  btn2.tick();
  btn3.tick();

  btn12.tick(btn1, btn2);
  btn13.tick(btn1, btn3);
  btn23.tick(btn2, btn3);

  if (btn1.hold())
  {
    SerialUSB.println("all on");
    setStatusPixel(statusPixel.Color(0, 0, 150));
    onAllLeds();
    if (!passive)
    {
      uint8_t data[] = {1};
      broadcast(SET_ALL_LEDS_reg, data, SET_ALL_LEDS_len);
    }
  }
  else if (btn1.click())
  {
    SerialUSB.println("toggle");
    setStatusPixel(statusPixel.Color(random(120, 255), random(120, 255), random(120, 255)));
    clearStatusPixel(1000);
    setEnabled(!enabled);
    if (!passive)
    {
      uint8_t data[] = {enabled ? 1 : 0};
      broadcast(SET_ENABLED_reg, data, SET_ENABLED_len);
    }
  }
  else if (btn2.click())
  {
    shuffle();
    SerialUSB.println("shuffle");
  }
  else if (btn2.hold())
  {
    shuffle();
    SerialUSB.println("shuffle all");
    if (!passive)
    {
      broadcast(SET_SHUFFLE_reg, nullptr, SET_SHUFFLE_len);
    }
  }
  else if (btn3.click())
  {
    // readAndWriteVar();
    // readAndWritePage(CONFIG_ADDR, 8);
    // config = getConfig();
    // return;
    auto oldConfig = config;
    config = config ^ 1 << 7;
    SerialUSB.print(oldConfig, BIN);
    SerialUSB.print(" -> ");
    SerialUSB.println(config, BIN);
    config = saveConfig(config);
    if (oldConfig != config)
    {
      SerialUSB.println("Config changed.");

      // uint8_t data[] = {config};
      // broadcast(SET_CONFIG_reg, data, SET_CONFIG_len);
    }
    else
    {
      SerialUSB.println("Config not changed.");
      error = true;
      setErrorPixel();
    }

    applyConfig();
    // clearEEPROM();
  }
  else if (btn3.hold())
  {
    // clearEEPROM();
    // dumpEEPROM();
    externalWire.end();
    externalWire.begin();
    scan(externalWire);
    return;

    // config = 0;
    // config = saveConfig(config);
    // SerialUSB.println(config, BIN);
    // SerialUSB.println("Config resetted.");

    // externalWire.end();
    // externalWire.begin();
    // scan(externalWire);
    // externalWire.end();
    // externalWire.begin(ext_addr);
    // externalWire.onReceive(I2C_RxHandler);

    readAndWriteVar();
    readAndWritePage(8, 2);
    dumpEEPROM();
  }
  else if (btn12.click())
  {
    SerialUSB.println("12");
  }
  else if (btn13.click())
  {
    SerialUSB.println("13");
  }
  else if (btn23.click())
  {
    SerialUSB.println("23");
    clearEEPROM();
  }
}

Task bhTask(10, TASK_FOREVER, &buttonHandler, &runner);

void setup()
{
  SerialUSB.begin(115200);
  // while (!SerialUSB)
  // {
  // }
  if (!SerialUSB)
  {
    delay(3000);
  }
  SerialUSB.println("LegoLight starting...");
  SerialUSB.print("Version: 0x0");
  SerialUSB.println(VERSION, HEX);

  uint32_t pins[] = {PIN_PA18, PIN_PA19};
  ext_addr = getAddress(I2C_ADDR_BASE, pins);
  randomSeed(analogRead(0) + ext_addr);
  SerialUSB.print("External address: 0x");
  SerialUSB.println(ext_addr, HEX);

  internalWire.begin();
  initExternalWire();

  if (!initEEPROM())
  {
    error = true;
    SerialUSB.println("EEPROM error");
  }
  else
  {
    config = getConfig();
  }
  SerialUSB.print("Config: 0b");
  SerialUSB.println(config, BIN);

  if (!initLedDriver())
  {
    SerialUSB.println("LED Driver error");
    error = true;
  }

  initPixels();
  if (error)
  {
    setErrorPixel();
    SerialUSB.println("LegoLight error");
    return;
  }

  clearStatusPixel(3000);

  bhTask.enable();
  SerialUSB.println("LegoLight inited.");
  clearLeds();
}

void loop()
{
  if (error)
  {
    setErrorPixel();
    SerialUSB.println("LegoLight error");
    delay(1000);
    return;
  }

  runner.execute();
}
