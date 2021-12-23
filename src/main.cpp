#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "heap.h"
#include "effects.h"

#define PIXEL_DATA_PIN  12
#define BUTTON_INTERRUPT_PIN    2
#define COLOR_CYCLE_BUTTON_PIN  4
#define EFFECT_BUTTON_PIN       6
#define BRIGHTNESS_BUTTON_PIN   8
#define BUTTON_DEBOUNCE_COOLDOWN_MS 40

volatile unsigned long brightness_button_debounce_alarm, color_button_debounce_alarm, effect_button_debounce_alarm;

Adafruit_NeoPixel Stage;

extern struct effect_s StaticColorEffect;
extern struct effect_s OrbitEffect;
struct effect_s * Effects[] =
{
    &StaticColorEffect,
    &OrbitEffect
};
uint8_t CurrentEffectIndex = 0;

const int BrightnessLevels[] = {0, 12, 30, 50, 180, 255};
uint8_t BrightnessIndex = 1;

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
        Stage.setBrightness(BrightnessLevels[BrightnessIndex]);
        delay(1000);
        Serial.println("Setup complete");
    }
    else
    {
        while(1);
    }
    brightness_button_debounce_alarm = color_button_debounce_alarm = effect_button_debounce_alarm = 0;
}

void loop()
{
    static unsigned long last_millis = 0;
    unsigned long current_millis = millis();

    const unsigned long brightness_alarm = brightness_button_debounce_alarm;
    const unsigned long color_alarm = color_button_debounce_alarm;
    const unsigned long effect_alarm = effect_button_debounce_alarm;

    if(brightness_alarm != 0 && brightness_alarm <= current_millis)
    {
        if(!digitalRead(BRIGHTNESS_BUTTON_PIN))
        {
            Serial.println("Changing Brightness...");
            if(++BrightnessIndex >= sizeof(BrightnessLevels) / sizeof(const int))
            {
                BrightnessIndex = 0;
            }
            Stage.setBrightness(BrightnessLevels[BrightnessIndex]);
        }
        brightness_button_debounce_alarm = 0;
    }
    if(color_alarm != 0 && color_alarm <= current_millis)
    {
        if(!digitalRead(COLOR_CYCLE_BUTTON_PIN))
        {
            Serial.println("Changing Color...");
            if(Effects[CurrentEffectIndex]->changeColor != NULL)
                Effects[CurrentEffectIndex]->changeColor();
        }
        color_button_debounce_alarm = 0;
    }
    if(effect_alarm != 0 && effect_alarm <= current_millis)
    {
        if(!digitalRead(EFFECT_BUTTON_PIN))
        {
            Serial.println("Changing Effect...");
            Stage.clear();
            if(++CurrentEffectIndex >= sizeof(Effects) / sizeof(struct effect_s *))
            {
                CurrentEffectIndex = 0;
            }
            Effects[CurrentEffectIndex]->init(&Stage);
            last_millis = current_millis;
            current_millis = millis();
        }
        effect_button_debounce_alarm = 0;
    }

    Stage.clear();
    Effects[CurrentEffectIndex]->step(current_millis - last_millis);
    Stage.show();

    last_millis = current_millis;
}

void handleButtons()
{
    unsigned long alarm_time = millis() + BUTTON_DEBOUNCE_COOLDOWN_MS;
    if(!digitalRead(BRIGHTNESS_BUTTON_PIN))
    {
        brightness_button_debounce_alarm = alarm_time;
    }
    else if(!digitalRead(COLOR_CYCLE_BUTTON_PIN))
    {
        color_button_debounce_alarm = alarm_time;
    }
    else if(!digitalRead(EFFECT_BUTTON_PIN))
    {
        effect_button_debounce_alarm = alarm_time;
    }
    //Serial.println("Button press detected.");
}

void setupButtons()
{
    pinMode(BRIGHTNESS_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COLOR_CYCLE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(EFFECT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUTTON_INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT_PIN), handleButtons, FALLING);
}