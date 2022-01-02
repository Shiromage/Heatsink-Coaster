// orbit.cpp
// Chase Baker

#include "effects.h"

#define ORB_COUNT   3
#define ORB_ANGULAR_SPEED   25
#define ORB_SIZE    75
#define ORB_SPACING (360 / ORB_COUNT)
#define BACKGROUND_LIGHT_INTENSITY  35

void initOrbitEffect(Adafruit_NeoPixel * stage);
void changeOrbitColor();
void stepOrbit(unsigned long);

uint8_t calc_brightness(int8_t bucket_index, float position);
void paint_bucket(int8_t led, uint8_t brightness);

struct effect_s OrbitEffect =
{
    .init = initOrbitEffect,
    .step = stepOrbit,
    .changeColor = changeOrbitColor
};

static Adafruit_NeoPixel * orbit_stage;

static uint32_t orb_color_index = 0;
static uint32_t background_fill_color;

typedef struct
{
    float angular_position;
} orb_s;

static orb_s Orbs[ORB_COUNT];

void initOrbitEffect(Adafruit_NeoPixel * pixels)
{
    orbit_stage = pixels;
    Orbs[0].angular_position = 0;
    uint32_t red, green, blue;
    red = SystemColors[orb_color_index] & 0x00FF0000;
    green = SystemColors[orb_color_index] & 0x0000FF00;
    blue = SystemColors[orb_color_index] & 0xFF;
    red = red >> 16;
    green = green >> 8;

    red = (red * BACKGROUND_LIGHT_INTENSITY) / 255.0; //Scale element with brightness
    green = (green * BACKGROUND_LIGHT_INTENSITY) / 255.0;
    blue = (blue * BACKGROUND_LIGHT_INTENSITY) / 255.0;
    background_fill_color = Adafruit_NeoPixel::Color(red, green, blue);
}

void changeOrbitColor()
{
    if(++orb_color_index >= COLOR_COUNT)
    {
        orb_color_index = 0;
    }
    uint32_t red, green, blue;
    red = SystemColors[orb_color_index] & 0x00FF0000;
    green = SystemColors[orb_color_index] & 0x0000FF00;
    blue = SystemColors[orb_color_index] & 0xFF;
    red = red >> 16;
    green = green >> 8;

    red = (red * BACKGROUND_LIGHT_INTENSITY) / 255.0; //Scale element with brightness
    green = (green * BACKGROUND_LIGHT_INTENSITY) / 255.0;
    blue = (blue * BACKGROUND_LIGHT_INTENSITY) / 255.0;
    background_fill_color = Adafruit_NeoPixel::Color(red, green, blue);
}

void stepOrbit(unsigned long millis)
{
    Orbs[0].angular_position += (ORB_ANGULAR_SPEED * millis / 1000.0);
    for(int orb_number = 1; orb_number < ORB_COUNT; orb_number++)
    {
        Orbs[orb_number].angular_position = Orbs[orb_number-1].angular_position + ORB_SPACING;
    }
    orbit_stage->fill(background_fill_color);

    for(uint8_t orb_number = 0; orb_number < ORB_COUNT; orb_number++)
    {
        int8_t head_bucket = (Orbs[orb_number].angular_position + ORB_SIZE / 2.0) / 360.0 * PIXEL_COUNT;
        float tail_position = Orbs[orb_number].angular_position - ORB_SIZE / 2.0;
        int8_t tail_bucket = tail_position / 360.0 * PIXEL_COUNT + ((tail_position < 0) ? -1 : 0); //Negative numbers need to be rounded down since positive numbers are also rounded down.
        for(int8_t bucket_index = head_bucket; bucket_index != tail_bucket; bucket_index--)
        {
            uint8_t brightness = calc_brightness(bucket_index, Orbs[orb_number].angular_position);
            paint_bucket(bucket_index, brightness);
        }
        paint_bucket(tail_bucket, calc_brightness(tail_bucket, Orbs[orb_number].angular_position));
    }
    while(Orbs[0].angular_position >= 360.0)
    {
        Orbs[0].angular_position -= 360.0;
    }
}

uint8_t calc_brightness(int8_t bucket_index, float angular_position)
{
    const float bucket_bisect_point = (bucket_index + 0.5) * (360.0 / PIXEL_COUNT); // Ex: Bucket 0 has a center point position of 7.5 degrees
    if(abs(bucket_bisect_point - angular_position) >= (ORB_SIZE / 2.0))
    {
        return BACKGROUND_LIGHT_INTENSITY;
    }
    else
    {
        return (uint8_t)map(abs(bucket_bisect_point - angular_position), 0.0, ORB_SIZE / 2, 255, BACKGROUND_LIGHT_INTENSITY);
    }
}

void paint_bucket(int8_t bucket_index, uint8_t brightness)
{
    while(bucket_index >= PIXEL_COUNT)
        bucket_index -= PIXEL_COUNT;
    while(bucket_index < 0)
        bucket_index += PIXEL_COUNT;
    uint32_t red, green, blue;
    red = SystemColors[orb_color_index] & 0x00FF0000;
    green = SystemColors[orb_color_index] & 0x0000FF00;
    blue = SystemColors[orb_color_index] & 0xFF;

    red = red >> 16;
    green = green >> 8;

    red = (red * brightness) / 255; //Scale element with brightness
    green = (green * brightness) / 255;
    blue = (blue * brightness) / 255;

    orbit_stage->setPixelColor(bucket_index, Adafruit_NeoPixel::Color(red, green, blue));
}
