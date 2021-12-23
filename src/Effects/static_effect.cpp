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

uint32_t static_colors[] =
{
    Adafruit_NeoPixel::Color(255, 255, 255),
    Adafruit_NeoPixel::Color(255, 200, 0),
    Adafruit_NeoPixel::Color(255, 0, 0),
    Adafruit_NeoPixel::Color(0, 255, 0),
    Adafruit_NeoPixel::Color(0, 0, 255),
    Adafruit_NeoPixel::Color(0, 255, 255),
    Adafruit_NeoPixel::Color(255, 0, 255)
};
uint8_t current_color_index = 0;

void initStaticEffect(Adafruit_NeoPixel * pixels)
{
    stage = pixels;
    Serial.println("Static setup");
}

void stepStaticEffect(unsigned long millis)
{
    stage->fill(static_colors[current_color_index]);
}

void changeStaticColor()
{
    if(++current_color_index == sizeof(static_colors) / sizeof(uint32_t))
    {
        current_color_index = 0;
    }
    Serial.print("color index is now ");
    Serial.println(current_color_index);
}
