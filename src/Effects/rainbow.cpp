#include "effects.h"

#define DEG_TO_HUE(_deg)    ((uint16_t)(_deg / 360.0 * 65535))

void initRainbow(Adafruit_NeoPixel *);
void stepRainbow(unsigned long);
void changeSpeed();

struct effect_s RainbowEffect =
{
    .init = initRainbow,
    .step = stepRainbow,
    .changeColor = changeSpeed
};

static Adafruit_NeoPixel * rainbow_stage;

static float position;
static uint32_t spin_speeds[] = //360 for each revolution
{
    15,
    30,
    80,
    200,
    720
};
uint8_t spin_speed_index = 0;

void initRainbow(Adafruit_NeoPixel * pixels)
{
    position = 0;
    spin_speed_index = 0;
    rainbow_stage = pixels;
}

void stepRainbow(unsigned long millis)
{
    position = position + (spin_speeds[spin_speed_index] * millis / 1000.0);

    for(uint8_t pixel = 0; pixel < rainbow_stage->numPixels(); pixel++)
    {
        float pixel_position = position + (360.0 * pixel / rainbow_stage->numPixels());
        uint32_t color = Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(DEG_TO_HUE(pixel_position)));
        rainbow_stage->setPixelColor(pixel, color);
    }
}

void changeSpeed()
{
    if(++spin_speed_index >= sizeof(spin_speeds) / sizeof(spin_speeds[0]))
    {
        spin_speed_index = 0;
    }
}
