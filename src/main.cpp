#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "heap.h"
#include "effects.h"

#define PIXEL_DATA_PIN  12
#define COLOR_CYCLE_BUTTON_PIN  5
#define EFFECT_BUTTON_PIN       7
#define BRIGHTNESS_BUTTON_PIN   3
#define BUTTON_DEBOUNCE_COOLDOWN_MS 200

Adafruit_NeoPixel Stage;

extern struct effect_s StaticColorEffect;
struct effect_s * Effects[] =
{
    &StaticColorEffect
};
uint8_t CurrentEffectIndex = 0;

const int BrightnessLevels[] = {0, 15, 45, 100, 180, 255};
uint8_t BrightnessIndex = 0;

void setupButtons();

void setup()
{
    //Determine if power from 9V is available. If not, do not power pixels.
    Serial.begin(9600);
    if(1)
    {
        Stage = *(new Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800));
        setupButtons();
        Stage.begin();
        Effects[CurrentEffectIndex]->init(&Stage);
        Serial.println("Setup complete");
    }
    else
    {
        while(1);
    }
}

volatile bool changeEffectFlag = false;
volatile bool changeBrightnessFlag = false;
volatile bool changeColorFlag = false;
long buttonCooldown = 0;
void loop()
{
    static unsigned long last_millis = 0;

    //Poll button pins for changes if no interrupt is attached to them.
    #if (digitalPinToInterrupt(BRIGHTNESS_BUTTON_PIN) == NOT_AN_INTERRUPT)
    changeBrightnessFlag = !digitalRead(BRIGHTNESS_BUTTON_PIN);
    #endif
    #if (digitalPinToInterrupt(COLOR_CYCLE_BUTTON_PIN) == NOT_AN_INTERRUPT)
    changeColorFlag = !digitalRead(COLOR_CYCLE_BUTTON_PIN);
    #endif
    #if (digitalPinToInterrupt(EFFECT_BUTTON_PIN) == NOT_AN_INTERRUPT)
    changeEffectFlag = !digitalRead(EFFECT_BUTTON_PIN);
    #endif

    if(changeBrightnessFlag)
    {
        Serial.println("Changing Brightness...");
        if(++BrightnessIndex >= sizeof(BrightnessLevels) / sizeof(const int))
        {
            BrightnessIndex = 0;
        }
        changeBrightnessFlag = false;
    }
    if(changeColorFlag)
    {
        Serial.println("Changing Color...");
        if(Effects[CurrentEffectIndex]->changeColor != NULL)
            Effects[CurrentEffectIndex]->changeColor();
        changeColorFlag = false;
    }
    if(changeEffectFlag)
    {
        Serial.println("Changing Effect...");
        Stage.clear();
        if(++CurrentEffectIndex >= sizeof(Effects) / sizeof(struct effect_s *))
        {
            CurrentEffectIndex = 0;
        }
        Effects[CurrentEffectIndex]->init(&Stage);
        last_millis = millis();
        changeEffectFlag = false;
        return;
    }

    unsigned long current_millis = millis();

    Stage.clear();
    Effects[CurrentEffectIndex]->step(last_millis - current_millis);
    Stage.show();

    last_millis = current_millis;
}

void changeBrightness()
{
    changeBrightnessFlag = true;
}

void changeColor()
{
    changeColorFlag = true;
}

void changeEffect()
{
    changeEffectFlag = true;
}

void setupButtons()
{
    pinMode(BRIGHTNESS_BUTTON_PIN, INPUT);
    pinMode(COLOR_CYCLE_BUTTON_PIN, INPUT);
    pinMode(EFFECT_BUTTON_PIN, INPUT);
    #if (digitalPinToInterrupt(BRIGHTNESS_BUTTON_PIN) != NOT_AN_INTERRUPT)
    attachInterrupt(digitalPinToInterrupt(BRIGHTNESS_BUTTON_PIN), changeBrightness, FALLING);
    #endif
    #if (digitalPinToInterrupt(COLOR_CYCLE_BUTTON_PIN) != NOT_AN_INTERRUPT)
    attachInterrupt(digitalPinToInterrupt(COLOR_CYCLE_BUTTON_PIN), changeColor, FALLING);
    #endif
    #if (digitalPinToInterrupt(EFFECT_BUTTON_PIN) != NOT_AN_INTERRUPT)
    attachInterrupt(digitalPinToInterrupt(EFFECT_BUTTON_PIN), changeEffect, FALLING);
    #endif
}