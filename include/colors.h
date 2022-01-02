// colors.h
// Chase Baker 2022

// Provides a set of gamma-corrected colors by name (enum color_name)

#ifndef _COLORS_H
#define _COLORS_H

#include <Adafruit_NeoPixel.h>

#define HUE_MAX 65535

enum color_name
{
    COLOR_RED,
    COLOR_ORANGE,
    COLOR_YELLOW,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_PURPLE,
    COLOR_WHITE,
    COLOR_COUNT
};

extern const uint16_t SystemHues[COLOR_COUNT];

extern const uint32_t SystemColors[COLOR_COUNT];

#endif //_COLORS_H