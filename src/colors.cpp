// colors.cpp
// Chase Baker 2022

#include "colors.h"

const uint16_t SystemHues[COLOR_COUNT] =
{
    0 * (HUE_MAX / 360), //Red
    27 * (HUE_MAX / 360), //Orange
    42 * (HUE_MAX / 360), //Yellow
    95 * (HUE_MAX / 360), //Green
    215 * (HUE_MAX / 360), //Blue
    310 * (HUE_MAX / 360), //Purple
    0   //White
};

const uint32_t SystemColors[COLOR_COUNT] =
{
    Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(SystemHues[COLOR_RED], 255, 255)), //Red
    Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(SystemHues[COLOR_ORANGE], 255, 255)), //Orange
    Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(SystemHues[COLOR_YELLOW], 255, 255)), //Yellow
    Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(SystemHues[COLOR_GREEN], 255, 255)), //Green
    Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(SystemHues[COLOR_BLUE], 255, 255)), //Blue
    Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(SystemHues[COLOR_PURPLE], 255, 220)),  //Purple
    Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(SystemHues[COLOR_WHITE], 0, 255))  //White
};