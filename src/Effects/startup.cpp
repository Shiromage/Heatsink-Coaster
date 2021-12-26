#include "effects.h"

#define ORB_COUNT   6
//Size in LEDs
#define ORB_SIZE    1

#define ORB_POSITION_ONE_REV    65535

#define OPACITY_MAX 255
#define OPACITY_TWINKLE_HIGH    OPACITY_MAX
#define OPACITY_TWINKLE_LOW 50

extern void initStartup(Adafruit_NeoPixel *);
extern void stepStartup(unsigned long);

struct effect_s StartupEffect =
{
    .init = initStartup,
    .step = stepStartup,
    .changeColor = NULL
};

Adafruit_NeoPixel * Pixels;

const uint32_t OrbColors[ORB_COUNT] =
{
    Adafruit_NeoPixel::Color(255, 0 ,0), //Red
    Adafruit_NeoPixel::Color(255, 220, 0), //Yellow
    Adafruit_NeoPixel::Color(0, 255, 0), //Green
    Adafruit_NeoPixel::Color(0, 220, 220), //Cyan
    Adafruit_NeoPixel::Color(0, 0, 255), //Blue
    Adafruit_NeoPixel::Color(190, 0, 190)  //Purple
};

enum startup_stage
{
    STAGE_INTRO,
    STAGE_SLOWDOWN,
    STAGE_DELAY1,
    STAGE_TWINKLE,
    STAGE_DELAY2,
    STAGE_TWIST_AND_FADE,
    STAGE_END
};

#define STAGE_INTRO_FADE_IN_SPEED   70
#define STAGE_INTRO_ORB_SPEED       270

#define STAGE_SLOWDOWN_ACCEL    (-40)

#define STAGE_DELAY1_MS 1000
#define STAGE_DELAY2_MS 600

enum startup_stage CurrentStage;

typedef struct
{
    int32_t angle_position; //[0 to 65535) for one revolution
    uint32_t color;
    uint8_t opacity; //aka brightness in HSV, 0 to 255
    uint8_t saturation; //0 to 255
} Orb;

Orb Orbs[ORB_COUNT];

void rectifyOrbs();
void drawOrbs();
uint8_t convert_position_to_bucket(int32_t);
uint8_t convert_position_to_led(int32_t);
int32_t convert_led_to_position(uint8_t);

void initStartup(Adafruit_NeoPixel * pixels)
{
    Pixels = pixels;
    CurrentStage = STAGE_INTRO;
    memset(&Orbs, 0x00, sizeof(Orbs));
    for(int orb_index = 0; orb_index < ORB_COUNT; orb_index++)
    {
        Orbs[orb_index].angle_position = orb_index * ((uint16_t)0xFFFF) / ORB_COUNT;
        Orbs[orb_index].color = Adafruit_NeoPixel::Color(255, 255, 255); //White
        Orbs[orb_index].opacity = 0;
        Orbs[orb_index].saturation = 0;
    }
    Serial.println("Setup of startup sketch complete.");
}

void stepStartup(unsigned long millis)
{
    static unsigned long time = 0;
    time += millis;
    Orb * target_orb;

    switch(CurrentStage)
    {
        case STAGE_INTRO:
        {
            int8_t index = -1;
            do
            {
                index++;
                target_orb = &Orbs[index];
            }while(target_orb->opacity != 255 && index < ORB_COUNT);
            if(index >= ORB_COUNT)
            {
                CurrentStage = STAGE_SLOWDOWN;
                Serial.println("Entering STAGE_SLOWDOWN");
                break;
            }
            uint16_t opactity_change = millis * STAGE_INTRO_FADE_IN_SPEED / 1000.0;
            target_orb->opacity = (target_orb->opacity + opactity_change >= 0xFF) ? 
                255 : (target_orb->opacity + opactity_change);
            Orbs[0].angle_position += STAGE_INTRO_ORB_SPEED * millis / 1000;
            rectifyOrbs();
        }break;
        case STAGE_SLOWDOWN:
        {
            CurrentStage = STAGE_DELAY1;
        }break;
        case STAGE_DELAY1:
        {
            CurrentStage = STAGE_TWINKLE;
        }break;
        case STAGE_TWINKLE:
        {
            CurrentStage = STAGE_DELAY2;
        }break;
        case STAGE_DELAY2:
        {
            CurrentStage = STAGE_TWIST_AND_FADE;
        }break;
        case STAGE_TWIST_AND_FADE:
        {
            Serial.println("Entering STAGE_END");
            CurrentStage = STAGE_END;
        }break;
        default:
        {
            Pixels->fill(0); //All dark
        }
    }
    drawOrbs();
}

//Remove rounding errors on the position of all orbs by using relative position from first orb
//Orbs will be evenly distributed
void rectifyOrbs()
{
    const int32_t base_position = Orbs[0].angle_position;
    for(int orb_index = 1; orb_index < ORB_COUNT; orb_index++)
    {
        Orbs[orb_index].angle_position = base_position + orb_index * ((uint16_t)0xFFFF) / ORB_COUNT;
        while(Orbs[orb_index].angle_position < 0)
            Orbs[orb_index].angle_position += 65535;
        while(Orbs[orb_index].angle_position > 65534)
            Orbs[orb_index].angle_position -= 65535;
    }
}

void drawOrbs()
{
    for(int index = 0; index < ORB_COUNT; index++)
    {
        int32_t upper_bound = Orbs[index].angle_position + (ORB_SIZE * (65535.0 / PIXEL_COUNT) / 2.0);
        int32_t lower_bound = Orbs[index].angle_position - (ORB_SIZE * (65535.0 / PIXEL_COUNT) / 2.0);

        uint8_t led = convert_position_to_led(upper_bound);
        uint8_t end_led = convert_position_to_led(lower_bound);
        while(led != end_led)
        {
            int32_t led_position = convert_led_to_position(led);
            int16_t brightness = 255 * abs(led_position - Orbs[index].angle_position) / 
                (float)(upper_bound - lower_bound);
            Pixels->setPixelColor(led, Orbs[index].color);
            led--;
        }
        Pixels->setPixelColor(led, Orbs[index].color);
    }
}

uint8_t convert_position_to_bucket(int32_t position)
{
    return position * PIXEL_COUNT / 65535;
}

uint8_t convert_position_to_led(int32_t position)
{
    int16_t led = position * PIXEL_COUNT / 65535;
    while(led < 0)
        led += PIXEL_COUNT;
    while(led >= PIXEL_COUNT)
        led -= PIXEL_COUNT;
    return led;
}

int32_t convert_led_to_position(uint8_t led)
{
    return (65535.0 / PIXEL_COUNT) * led;
}
