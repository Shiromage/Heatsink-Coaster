//colors.h

#ifndef _COLORS_H
#define _COLORS_H

#include <Adafruit_NeoPixel.h>

enum brightness_level
{
    BRIGHTNESS_10,
    BRIGHTNESS_25,
    BRIGHTNESS_50,
    BRIGHTNESS_100
};

enum color_name
{
    COLOR_RED,
    COLOR_ORANGE,
    COLOR_YELLOW,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_BLUE,
    COLOR_PURPLE,
    COLOR_PINK,
    COLOR_WHITE
};

extern const unsigned int color_codes[] =
{
    0x0000FF00,
    0x00000000,
    0x00000000,
    0x00FF0000,
    0x00000000,
    0x000000FF,
    0x00000000,
    0x00000000,
    0x00EEEEEE
};

#endif //_COLORS_H