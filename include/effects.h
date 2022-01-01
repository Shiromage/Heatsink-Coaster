//effects.h

#ifndef _EFFECTS_H
#define _EFFECTS_H

#include "system.h"
#include <Adafruit_NeoPixel.h>

struct effect_s
{
    void (*init)(Adafruit_NeoPixel * stage);    //Initialize the effect. Pixel color assignments are done through stage. Don't use stage.show().
    void (*step)(unsigned long millis);                  //Draw next frame of effect. millis is the number of milliseconds since the last frame.
    void (*changeColor)();                      //Change the color(s) used by the effect. May leave as NULL if no color changes are possible.
};

#endif // _EFFECTS_H
