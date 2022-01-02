// static_effect.cpp
// Chase Baker 2022

#include "effects.h"

void initStaticEffect(Adafruit_NeoPixel *);
void stepStaticEffect(unsigned long);
void changeStaticColor();

struct effect_s StaticColorEffect =
{
    .init = initStaticEffect,
    .step = stepStaticEffect,
    .changeColor = changeStaticColor
};

static Adafruit_NeoPixel * stage;
uint8_t current_color_index = 0;

void initStaticEffect(Adafruit_NeoPixel * pixels)
{
    stage = pixels;
}

void stepStaticEffect(unsigned long millis)
{
    stage->fill(SystemColors[current_color_index]);
}

void changeStaticColor()
{
    if(++current_color_index == COLOR_COUNT)
    {
        current_color_index = 0;
    }
}
