// breathe.cpp

#include "effects.h"

#define BREATHE_INTERVAL_MS 12000
#define BREATHE_PHASOR_VELOCITY_MS (360.0 / BREATHE_INTERVAL_MS)
#define BREATH_LOW_VALUE    30
#define BREATH_HIGH_VALUE   255

void initBreathe(Adafruit_NeoPixel *);
void stepBreathe(unsigned long);
void colorBreathe();

struct effect_s BreatheEffect =
{
    .init = initBreathe,
    .step = stepBreathe,
    .changeColor = colorBreathe
};

Adafruit_NeoPixel * breathe_stage;
const uint32_t colors[] = 
{
    Adafruit_NeoPixel::Color(255, 255, 255),
    Adafruit_NeoPixel::Color(255, 0, 0),
    Adafruit_NeoPixel::Color(0, 255, 0),
    Adafruit_NeoPixel::Color(0, 0, 255),
    Adafruit_NeoPixel::Color(255, 255, 0),
    Adafruit_NeoPixel::Color(255, 130, 0)
};
uint8_t color_index = 0;
static float phasor_position;

void initBreathe(Adafruit_NeoPixel * pixels)
{
    breathe_stage = pixels;
    phasor_position = 0;
}

//The breathing animation is driven by a phasor
void stepBreathe(unsigned long millis)
{
    //from breathe interval, calculate phasor angular velocity.
    phasor_position += BREATHE_PHASOR_VELOCITY_MS * millis;
    while(phasor_position >= 360.0)
    {
        phasor_position -= 360.0;
    }
    
    float breath_position = sin(radians(phasor_position)); //-1 is start of inhale, 1 is start of exhale
    float breath_brightness = map(breath_position * 512, -512, 512, BREATH_LOW_VALUE, BREATH_HIGH_VALUE);

    uint32_t unscaled_color = colors[color_index];
    uint8_t red = unscaled_color >> 16;
    uint8_t green = unscaled_color >> 8;
    uint8_t blue = unscaled_color;
    red = red * breath_brightness / 255;
    green = green * breath_brightness / 255;
    blue = blue * breath_brightness / 255;

    breathe_stage->fill(Adafruit_NeoPixel::Color(red, green, blue));
}

void colorBreathe()
{
    if(++color_index >= sizeof(colors) / sizeof(colors[0]))
    {
        color_index = 0;
    }
}
