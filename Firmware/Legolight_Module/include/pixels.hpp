#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel externalPixels(EXT_PIXELS_COUNT, PIN_PA27, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel statusPixel(1, PIN_PA28, NEO_GRB + NEO_KHZ800);

uint8_t statusBrightness = 100;

uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return externalPixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return externalPixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return externalPixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

uint16_t j2;
void rainbowStatus()
{
    j2++;
    if (j2 >= 256)
        j2 = 0;

    statusPixel.setPixelColor(0, Wheel((j2) & 255));
    statusPixel.show();
}

uint16_t j;
void rainbow()
{
    j++;
    if (j >= 256)
        j = 0;
    uint16_t i;

    for (i = 0; i < externalPixels.numPixels(); i++)
    {
        externalPixels.setPixelColor(i, Wheel((i + j) & 255));
    }
    externalPixels.show();
    // rainbowStatus();
}

Task rainbowTask(5, TASK_FOREVER, &rainbow);

void stopExternalPixels()
{
    rainbowTask.disable();
    externalPixels.clear();
    externalPixels.show();

    statusPixel.clear();
    statusPixel.show();
}

void startExternalPixels()
{
    rainbowTask.enable();
}

void initPixels()
{
    externalPixels.begin();
    statusPixel.begin();
    statusPixel.setPixelColor(0, statusPixel.Color(0, statusBrightness, 0));
    statusPixel.show();

    runner.addTask(rainbowTask);
    applyConfig();
    SerialUSB.println("Pixels inited.");
}

void setStatusPixel(uint32_t color)
{
    SerialUSB.print("Setting status pixel to: ");
    SerialUSB.println(color);
    statusPixel.setPixelColor(0, color);
    statusPixel.show();
}

void setErrorPixel()
{
    setStatusPixel(statusPixel.Color(255, 0, 0));
}

void clearStatusPixel(int delayMs = 0)
{
    SerialUSB.print("Clearing status pixel in ");
    SerialUSB.println(delayMs);
    Task *t = new Task(
        0, 1,
        []()
        {
            statusPixel.clear();
            statusPixel.show();
            SerialUSB.println("Status off.");
        },
        &runner);
    t->enableDelayed(delayMs);
}
