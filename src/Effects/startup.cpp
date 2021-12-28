#include "effects.h"

#define ORB_COUNT   6
//Size in LEDs
#define ORB_SIZE    1

//Ticks per revolution
#define ORB_POSITION_ONE_REV    65535

#define OPACITY_MAX 65535
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
    STAGE_INTRO,            // White orbs fade in, orbiting the center
    STAGE_SLOWDOWN,         // The orbs slow down to a stop, with Orbs[0] being at the 90 degree position
    STAGE_DELAY1,           // Delay
    STAGE_TWINKLE,          // The orbs change color, one after the other, briefly changing brightness as they do.
    STAGE_DELAY2,           // Delay
    STAGE_TWIST_AND_FADE,   // All orbs accelerate quickly while simultaneously fading out.
    STAGE_END
};

#define STAGE_INTRO_FADE_IN_SPEED   24000
#define STAGE_INTRO_ORB_SPEED       40000
#define STAGE_INTRO_MAX_OPACITY     20000

#define ORB_0_STOP_POSITION     (ORB_POSITION_ONE_REV / 4)
#define STAGE_SLOWDOWN_ACCEL    (-30000)

#define STAGE_DELAY1_MS 1000

#define STAGE_DELAY2_MS 600

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
            index_orb = -1;
            do
            {
                index_orb++;
                target_orb = &Orbs[index_orb];
            }while((target_orb->opacity == STAGE_INTRO_MAX_OPACITY) && (index_orb < ORB_COUNT));
            if(index_orb >= ORB_COUNT)
            {
                CurrentStage = STAGE_SLOWDOWN;
                Serial.println("Entering STAGE_SLOWDOWN");
                break;
            }
            uint32_t opacity_change = delta_micros * STAGE_INTRO_FADE_IN_SPEED / 1000000;
            //uint32_t overflow = (target_orb->opacity + opactity_change) % OPACITY_MAX;
            target_orb->opacity = (target_orb->opacity + opacity_change >= STAGE_INTRO_MAX_OPACITY) ?
                STAGE_INTRO_MAX_OPACITY : (target_orb->opacity + opacity_change);
            Orbs[0].angle_position += STAGE_INTRO_ORB_SPEED * delta_micros / 1000000;
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
                Serial.println(" Entering STAGE_DELAY1");
                CurrentStage = STAGE_DELAY1;
            }
            rectifyOrbs();
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
    last_micros = current_micros;
}

//Remove rounding errors on the position of all orbs by using relative position from first orb
//Orbs will be evenly distributed
void rectifyOrbs()
{
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

        int32_t upper_bound = Orbs[index].angle_position + (ORB_SIZE * (65535.0 / PIXEL_COUNT) / 2.0);
        int32_t lower_bound = Orbs[index].angle_position - (ORB_SIZE * (65535.0 / PIXEL_COUNT) / 2.0);

        uint8_t bucket = convert_position_to_bucket(upper_bound);
        uint8_t end_bucket = convert_position_to_bucket(lower_bound);
        int32_t led_position;
        while(bucket != end_bucket)
        {
            led_position = convert_led_to_position(bucket);
            int16_t brightness = OPACITY_MAX * abs(led_position - Orbs[index].angle_position) /
                (float)(upper_bound - lower_bound);
            Pixels->setPixelColor(convert_position_to_led(led_position), Orbs[index].color);
            bucket--;
        }
        led_position = convert_led_to_position(bucket);
        Pixels->setPixelColor(convert_position_to_led(led_position), Orbs[index].color);
    }
}

uint8_t convert_position_to_bucket(int32_t position)
{
    return position * PIXEL_COUNT / ORB_POSITION_ONE_REV;
}

uint8_t convert_position_to_led(int32_t position)
{
    int16_t led = position * PIXEL_COUNT / ORB_POSITION_ONE_REV;
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
