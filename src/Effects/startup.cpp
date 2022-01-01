#include "effects.h"

#define ORB_COUNT   6
//Size in LEDs
#define ORB_SIZE    2.2

//Ticks per revolution
#define ORB_POSITION_ONE_REV    65535
#define OPACITY_MAX 65535

#define HUE_MAX 65535
#define SATURATION_MAX  255
#define BRIGHTNESS_MAX  255

extern void initStartup(Adafruit_NeoPixel *);
extern void stepStartup(unsigned long);

struct effect_s StartupEffect =
{
    .init = initStartup,
    .step = stepStartup,
    .changeColor = NULL
};

Adafruit_NeoPixel * Pixels;

const uint16_t OrbHues[ORB_COUNT] =
{
    0 * (HUE_MAX / 360), //Red
    27 * (HUE_MAX / 360), //Orange
    42 * (HUE_MAX / 360), //Yellow
    95 * (HUE_MAX / 360), //Green
    215 * (HUE_MAX / 360), //Blue
    310 * (HUE_MAX / 360), //Purple
};

const uint32_t OrbColors[ORB_COUNT] =
{
    Adafruit_NeoPixel::ColorHSV(OrbHues[0], 255, 255), //Red
    Adafruit_NeoPixel::ColorHSV(OrbHues[1], 255, 255), //Yellow
    Adafruit_NeoPixel::ColorHSV(OrbHues[2], 255, 255), //Orange
    Adafruit_NeoPixel::ColorHSV(OrbHues[3], 255, 255), //Green
    Adafruit_NeoPixel::ColorHSV(OrbHues[4], 255, 255), //Blue
    Adafruit_NeoPixel::ColorHSV(OrbHues[5], 255, 220)  //Purple
};

enum startup_stage
{
    STAGE_INTRO,            // White orbs fade in, orbiting the center
    STAGE_SEEK,             // Search for the point at which slowdown begins
    STAGE_SLOWDOWN,         // The orbs slow down to a stop, with Orbs[0] being at the 90 degree position
    STAGE_DELAY1,           // Delay
    STAGE_TWINKLE,          // The orbs change color, one after the other, briefly changing brightness as they do.
    STAGE_DELAY2,           // Delay
    STAGE_TWIST_AND_FADE,   // All orbs accelerate quickly while simultaneously fading out.
    STAGE_END
};

#define STAGE_INTRO_FADE_IN_SPEED   20000
#define STAGE_INTRO_ORB_SPEED       40000
#define STAGE_INTRO_MAX_OPACITY     18500

#define STAGE_SLOWDOWN_STOP_POSITION     (ORB_POSITION_ONE_REV * 263.5 / 360)
#define STAGE_SLOWDOWN_ACCEL    (-30000)
#define STAGE_SLOWDOWN_DISTANCE_UNTIL_STOP  (STAGE_INTRO_ORB_SPEED * ((float)STAGE_INTRO_ORB_SPEED / (float)abs(STAGE_SLOWDOWN_ACCEL)) / 2)

#define STAGE_DELAY1_MS 1000

#define STAGE_TWINKLE_OPACITY_HIGH  OPACITY_MAX
#define STAGE_TWINKLE_OPACITY_LOW   (STAGE_TWINKLE_OPACITY_HIGH * 7/16)
#define STAGE_TWINKLE_TT_OPACITY_HIGH_MS    100
#define STAGE_TWINKLE_TT_OPACITY_LOW_MS     1200
#define STAGE_TWINKLE_CONSECUTIVE_ORB_DELAY 300

#define STAGE_DELAY2_MS 1600

#define STAGE_TWIST_AND_FADE_ACCEL  450000
#define STAGE_TWIST_AND_FADE_TT_ZERO_OPACITY_MS 400

enum startup_stage CurrentStage;

typedef struct
{
    int32_t angle_position; //[0 to 65535) for one revolution
    int32_t velocity; //position-ticks per second
    uint32_t color; //This is used to draw Orbs; opacity and saturation are just state variables
    uint16_t opacity; //aka brightness in HSV, 0 to 65535
    uint8_t saturation; //0 to 255
} Orb;

Orb Orbs[ORB_COUNT];

void rectifyOrbs();
void drawOrbs();
uint32_t apply_brightness(uint8_t, uint32_t);
uint32_t angular_distance(int32_t, int32_t);
uint8_t convert_position_to_bucket(int32_t);
int32_t convert_led_to_position(uint8_t);
uint8_t convert_bucket_to_led(int32_t);

void initStartup(Adafruit_NeoPixel * pixels)
{
    Pixels = pixels;
    CurrentStage = STAGE_INTRO;
    memset(&Orbs, 0x00, sizeof(Orbs));
    for(int orb_index = 0; orb_index < ORB_COUNT; orb_index++)
    {
        Orbs[orb_index].angle_position = orb_index * ORB_POSITION_ONE_REV / ORB_COUNT;
        Orbs[orb_index].velocity = STAGE_INTRO_ORB_SPEED;
        Orbs[orb_index].color = Adafruit_NeoPixel::Color(255, 255, 255); //White
        Orbs[orb_index].opacity = 0;
        Orbs[orb_index].saturation = 0;
    }
    Serial.println("Setup of startup sketch complete.");
}

void stepStartup(unsigned long millis)
{
    static unsigned long last_micros = 0;
    unsigned long current_micros = micros();
    if(last_micros == 0)
    {
        last_micros = current_micros - 1;
    }
    unsigned long delta_micros = current_micros - last_micros;
    Orb * target_orb;
    volatile int8_t index_orb; //Arduino compiler is shit, so I need this to be volatile. Without it, extremely unexpected behavior occurs.

    switch(CurrentStage)
    {
        case STAGE_INTRO:
        {
            index_orb = ORB_COUNT;
            do
            {
                index_orb--;
                target_orb = &Orbs[index_orb];
            }while((target_orb->opacity == STAGE_INTRO_MAX_OPACITY) && (index_orb >= 0));
            if(index_orb < 0)
            {
                CurrentStage = STAGE_SEEK;
                Serial.println("Entering STAGE_SEEK");
                break;
            }
            uint32_t opacity_change = delta_micros * STAGE_INTRO_FADE_IN_SPEED / 1000000;
            //uint32_t overflow = (target_orb->opacity + opactity_change) % OPACITY_MAX;
            target_orb->opacity = (target_orb->opacity + opacity_change >= STAGE_INTRO_MAX_OPACITY) ?
                STAGE_INTRO_MAX_OPACITY : (target_orb->opacity + opacity_change);
            Orbs[0].angle_position += STAGE_INTRO_ORB_SPEED * delta_micros / 1000000;
            rectifyOrbs();
        }break;
        case STAGE_SEEK:
        {
            Orbs[0].angle_position += Orbs[0].velocity * delta_micros / 1000000;
            static bool trigger = false;
            if(angular_distance(Orbs[0].angle_position, STAGE_SLOWDOWN_STOP_POSITION) <= STAGE_SLOWDOWN_DISTANCE_UNTIL_STOP)
            {
                if(trigger)
                {
                    trigger = false;
                    CurrentStage = STAGE_SLOWDOWN;
                    Serial.println("Entering STAGE_SLOWDOWN");
                }
            }
            else
            {
                trigger = true;
            }
            rectifyOrbs();
        }break;
        case STAGE_SLOWDOWN:
        {
            Orbs[0].angle_position += Orbs[0].velocity * delta_micros / 1000000;
            int32_t velocity_change = STAGE_SLOWDOWN_ACCEL * (signed long)delta_micros / 1000000;
            Orbs[0].velocity += (Orbs[0].velocity < 0) ? -velocity_change : velocity_change;
            if((Orbs[0].velocity < 0 && velocity_change < 0) || (Orbs[0].velocity >= 0 && velocity_change > 0))
            {
                Orbs[0].velocity = 0;
                Serial.println("Entering STAGE_DELAY1");
                CurrentStage = STAGE_DELAY1;
            }
            rectifyOrbs();
        }break;
        case STAGE_DELAY1:
        {
            delay(STAGE_DELAY1_MS);
            current_micros = micros();
            Serial.println("Entering STAGE_TWINKLE");
            CurrentStage = STAGE_TWINKLE;
        }break;
        case STAGE_TWINKLE:
        {
            uint32_t opacity_change;
            for(index_orb = 0; index_orb < ORB_COUNT; index_orb++)
            {
                if(Orbs[index_orb].saturation < SATURATION_MAX)
                {
                    uint32_t saturation_change = delta_micros * ((uint32_t)SATURATION_MAX) / ((uint32_t)STAGE_TWINKLE_TT_OPACITY_HIGH_MS * (uint32_t)1000UL);
                    opacity_change = delta_micros * (OPACITY_MAX - STAGE_INTRO_MAX_OPACITY) / (STAGE_TWINKLE_TT_OPACITY_HIGH_MS * 1000UL);
                    Orbs[index_orb].opacity = (opacity_change + Orbs[index_orb].opacity > OPACITY_MAX) ? OPACITY_MAX : (opacity_change + Orbs[index_orb].opacity);
                    Orbs[index_orb].saturation = (saturation_change + Orbs[index_orb].saturation > SATURATION_MAX) ? SATURATION_MAX : (saturation_change + Orbs[index_orb].saturation);
                    Orbs[index_orb].color = Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(OrbHues[index_orb], Orbs[index_orb].saturation, BRIGHTNESS_MAX));
                }
                else if(Orbs[index_orb].opacity > STAGE_TWINKLE_OPACITY_LOW)
                {
                    opacity_change = delta_micros * (OPACITY_MAX - STAGE_TWINKLE_OPACITY_LOW) / (STAGE_TWINKLE_TT_OPACITY_LOW_MS * 1000UL);
                    Orbs[index_orb].opacity = (Orbs[index_orb].opacity - opacity_change < STAGE_TWINKLE_OPACITY_LOW) ? STAGE_TWINKLE_OPACITY_LOW : (Orbs[index_orb].opacity - opacity_change);
                }
            }
            if(Orbs[ORB_COUNT-1].saturation == SATURATION_MAX && Orbs[ORB_COUNT-1].opacity == STAGE_TWINKLE_OPACITY_LOW)
            {
                Serial.println("Entering STAGE_DELAY2");
                CurrentStage = STAGE_DELAY2;
            }
        }break;
        case STAGE_DELAY2:
        {
            delay(STAGE_DELAY2_MS);
            current_micros = micros();
            CurrentStage = STAGE_TWIST_AND_FADE;
        }break;
        case STAGE_TWIST_AND_FADE:
        {
            Orbs[0].velocity += delta_micros * STAGE_TWIST_AND_FADE_ACCEL / 1000000;
            Orbs[0].angle_position += delta_micros * Orbs[0].velocity / 1000000;
            int32_t opacity_change = delta_micros * (STAGE_TWINKLE_OPACITY_LOW - 0) / (STAGE_TWIST_AND_FADE_TT_ZERO_OPACITY_MS * 1000UL);
            Orbs[0].opacity = (Orbs[0].opacity - opacity_change < 0) ? 0 : (Orbs[0].opacity - opacity_change);
            for(index_orb = 1; index_orb < ORB_COUNT; index_orb++)
            {
                Orbs[index_orb].opacity = Orbs[0].opacity;
            }
            if(Orbs[0].opacity == 0)
            {
                CurrentStage = STAGE_END;
                Serial.println("Entering STAGE_END");
                break;
            }
            rectifyOrbs();
        }break;
        default:
        {
            Pixels->fill(0); //All dark
        }
    }
    drawOrbs();
    last_micros = current_micros;
}

//Remove rounding errors on the position of all orbs by using relative position from first orb
//Orbs will be evenly distributed
void rectifyOrbs()
{
    while(Orbs[0].angle_position < 0)
        Orbs[0].angle_position += ORB_POSITION_ONE_REV;
    while(Orbs[0].angle_position > (ORB_POSITION_ONE_REV - 1))
        Orbs[0].angle_position -= ORB_POSITION_ONE_REV;
    const int32_t base_position = Orbs[0].angle_position;
    for(int orb_index = 1; orb_index < ORB_COUNT; orb_index++)
    {
        Orbs[orb_index].angle_position = base_position + orb_index * ORB_POSITION_ONE_REV / ORB_COUNT;
        while(Orbs[orb_index].angle_position < 0)
            Orbs[orb_index].angle_position += ORB_POSITION_ONE_REV;
        while(Orbs[orb_index].angle_position > (ORB_POSITION_ONE_REV - 1))
            Orbs[orb_index].angle_position -= ORB_POSITION_ONE_REV;
    }
}

void drawOrbs()
{
    for(int index = 0; index < ORB_COUNT; index++)
    {
        if(Orbs[index].opacity == 0)
        {
            continue;
        }

        int32_t upper_bound = Orbs[index].angle_position + (ORB_SIZE * (ORB_POSITION_ONE_REV / PIXEL_COUNT) / 2);
        int32_t lower_bound = Orbs[index].angle_position - (ORB_SIZE * (ORB_POSITION_ONE_REV / PIXEL_COUNT) / 2);

        uint8_t bucket = convert_position_to_bucket(upper_bound);
        uint8_t end_bucket = convert_position_to_bucket(lower_bound);
        int32_t led_position;
        uint32_t brightness, scaled_color;
        while(bucket != end_bucket)
        {
            led_position = convert_led_to_position(bucket);
            //The further away an led is from the center of the orb, the dimmer it is.
            if(abs(led_position - Orbs[index].angle_position) > (upper_bound - lower_bound) / 2)
            {
                brightness = 0;
            }
            else
            {
                brightness = map(abs(led_position - Orbs[index].angle_position), 0, (upper_bound - lower_bound) / 2, (uint32_t)Orbs[index].opacity * BRIGHTNESS_MAX / OPACITY_MAX, 0);
            }
            scaled_color = apply_brightness(brightness, Orbs[index].color);
            Pixels->setPixelColor(convert_bucket_to_led(bucket), scaled_color);
            bucket--;
        }
        led_position = convert_led_to_position(bucket);
        if(abs(led_position - Orbs[index].angle_position) > (upper_bound - lower_bound) / 2)
        {
            brightness = 0;
        }
        else
        {
            brightness = map(abs(led_position - Orbs[index].angle_position), 0, (upper_bound - lower_bound) / 2, (uint32_t)Orbs[index].opacity * BRIGHTNESS_MAX / OPACITY_MAX, 0);
        }
        scaled_color = apply_brightness(brightness, Orbs[index].color);
        Pixels->setPixelColor(convert_bucket_to_led(bucket), scaled_color);
    }
}

//Scale the brightness of a color by the brightness value given
uint32_t apply_brightness(uint8_t brightness, uint32_t base_color)
{
    uint32_t red, green, blue;
    red = base_color & 0x00FF0000;
    green = base_color & 0x0000FF00;
    blue = base_color & 0xFF;

    red = red >> 16;
    green = green >> 8;

    red = (red * brightness) / BRIGHTNESS_MAX; //Scale element with brightness
    green = (green * brightness) / BRIGHTNESS_MAX;
    blue = (blue * brightness) / BRIGHTNESS_MAX;

    return Adafruit_NeoPixel::Color(red, green, blue);
}

uint32_t angular_distance(int32_t from, int32_t to)
{
    if(from <= to)
    {
        return to - from;
    }
    else
    {
        return (ORB_POSITION_ONE_REV - from) + to;
    }
}

uint8_t convert_position_to_bucket(int32_t position)
{
    return position * PIXEL_COUNT / ORB_POSITION_ONE_REV;
}

int32_t convert_led_to_position(uint8_t led)
{
    return led * ORB_POSITION_ONE_REV / PIXEL_COUNT;
}

uint8_t convert_bucket_to_led(int32_t bucket)
{
    while(bucket < 0)
        bucket += PIXEL_COUNT;
    while(bucket >= PIXEL_COUNT)
        bucket -= PIXEL_COUNT;
    return bucket;
}
