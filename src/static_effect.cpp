#include "effects.h"

void initStaticEffect(Adafruit_NeoPixel *);
void stepStaticEffect(long);
void changeStaticColor();

struct effect_s StaticColorEffect =
{
    .init = initStaticEffect,
    .step = stepStaticEffect,
    .changeColor = changeStaticColor
};

static Adafruit_NeoPixel * stage;

unsigned int static_colors[] = {(unsigned int)0x0000FF00, (unsigned int)0x0030CC00, (unsigned int)0x0099FF00, (unsigned int)0x00FF0000, (unsigned int)0x0000AAAA, (unsigned int)0x000000FF, (unsigned int)0x00BB00BB, (unsigned int)0x0044FF44, (unsigned int)0x00CCCCCC};
uint8_t current_color_index = 0;

void initStaticEffect(Adafruit_NeoPixel * pixels)
{
    stage = pixels;
    Serial.println("Static setup");
}

void stepStaticEffect(long millis)
{
    stage->fill(static_colors[current_color_index]);
}

void changeStaticColor()
{
    if(++current_color_index == sizeof(static_colors) / sizeof(unsigned int))
    {
        current_color_index = 0;
    }
    Serial.print("color index is now ");
    Serial.println(current_color_index);
}
