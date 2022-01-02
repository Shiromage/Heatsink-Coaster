// main.cpp
// Chase Baker 2022

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "system.h"
#include "heap.h"
#include "effects.h"

#define MIN_LED_VOLTAGE 1.55
#define ANALOG_MAX_VALUE    1024
#define ANALOG_REF_VOLTAGE  5
#define POWER_DETECT_LED_REQ    (MIN_LED_VOLTAGE * ANALOG_MAX_VALUE / ANALOG_REF_VOLTAGE)

#define BUTTON_DEBOUNCE_COOLDOWN_MS 40

#define STARTUP_EFFECT_RUNTIME_MS  15000

#define POST_STARTUP_FADE_MS   700
#define STARTUP_OPACITY_CONTROL_DISABLE 255

volatile unsigned long brightness_button_debounce_alarm, color_button_debounce_alarm, effect_button_debounce_alarm;

Adafruit_NeoPixel Stage;

extern struct effect_s StartupEffect;
extern struct effect_s StaticColorEffect;
extern struct effect_s OrbitEffect;
extern struct effect_s RainbowEffect;
extern struct effect_s BreatheEffect;
struct effect_s * Effects[] =
{
    &StaticColorEffect,
    &OrbitEffect,
    &RainbowEffect,
    &BreatheEffect
};
uint8_t CurrentEffectIndex = 0;

const int BrightnessLevels[] = {0, 15, 40, 75, 150, 255};
static uint8_t BrightnessIndex = 3;
static uint16_t StartupOpacity;

static unsigned long last_millis;

void setupButtons();

void setup()
{
    Serial.begin(9600);
    pinMode(POWER_SUPPLY_DETECT_PIN, INPUT);
    analogReference(DEFAULT);
    int16_t power_detect_reading = analogRead(POWER_SUPPLY_DETECT_PIN);
    Serial.print("Power detect: "); Serial.print(power_detect_reading); Serial.print(" "); Serial.println((float)power_detect_reading * ANALOG_REF_VOLTAGE / ANALOG_MAX_VALUE);

    Stage = *(new Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800));
    Stage.begin();
    Stage.clear();
    Stage.show();

    Stage.setBrightness(255);
    StartupEffect.init(&Stage);
    unsigned long end_startup_time = millis();
    unsigned long current_time = end_startup_time;
    end_startup_time += STARTUP_EFFECT_RUNTIME_MS;
    do
    {
        unsigned long old_time = current_time;
        current_time = millis();
        Stage.clear();
        if(analogRead(POWER_SUPPLY_DETECT_PIN) < POWER_DETECT_LED_REQ) //Insufficient power for LEDs?
        {
            Stage.show();
            delay(5);
            continue;
        }
        StartupEffect.step(current_time - old_time);
        Stage.show();
    }while(current_time < end_startup_time);
    StartupOpacity = 0;

    brightness_button_debounce_alarm = color_button_debounce_alarm = effect_button_debounce_alarm = 0;
    setupButtons();
    Effects[CurrentEffectIndex]->init(&Stage);
    Stage.setBrightness(0);
    if(power_detect_reading < POWER_DETECT_LED_REQ)
    {
        Serial.print("Insufficient power for LEDs: "); Serial.println((float)power_detect_reading * ANALOG_REF_VOLTAGE / ANALOG_MAX_VALUE);
    }
    last_millis = millis();
}

void loop()
{
    unsigned long current_millis = millis();

    const unsigned long brightness_alarm = brightness_button_debounce_alarm;
    const unsigned long color_alarm = color_button_debounce_alarm;
    const unsigned long effect_alarm = effect_button_debounce_alarm;

    if(brightness_alarm != 0 && brightness_alarm <= current_millis)
    {
        if(!digitalRead(BRIGHTNESS_BUTTON_PIN))
        {
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
            if(Effects[CurrentEffectIndex]->changeColor != NULL)
                Effects[CurrentEffectIndex]->changeColor();
        }
        color_button_debounce_alarm = 0;
    }
    if(effect_alarm != 0 && effect_alarm <= current_millis)
    {
        if(!digitalRead(EFFECT_BUTTON_PIN))
        {
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
    if(StartupOpacity < BrightnessLevels[BrightnessIndex]) //If startup animation recently finished, fade in the first effect
    {
        StartupOpacity += (current_millis - last_millis) * 255U / POST_STARTUP_FADE_MS;
        if(StartupOpacity >= BrightnessLevels[BrightnessIndex])
        {
            StartupOpacity = STARTUP_OPACITY_CONTROL_DISABLE;
            Stage.setBrightness(BrightnessLevels[BrightnessIndex]);
        }
        else
        {
            Stage.setBrightness(StartupOpacity);
        }
        delay(1);
    }
    if(analogRead(POWER_SUPPLY_DETECT_PIN) < POWER_DETECT_LED_REQ) //Insufficient power for LEDs?
    {
        Stage.clear();
        Stage.show();
        do
        {
            Serial.println("Insufficient power for LEDs: ");
            delay(1000);
        }while(analogRead(POWER_SUPPLY_DETECT_PIN) < POWER_DETECT_LED_REQ);
        Effects[CurrentEffectIndex]->init(&Stage);
        current_millis = millis() - 1;
    }
    else
    {
        Effects[CurrentEffectIndex]->step(current_millis - last_millis);
        Stage.show();
    }
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
}

void setupButtons()
{
    pinMode(BRIGHTNESS_BUTTON_PIN, INPUT_PULLUP);
    pinMode(COLOR_CYCLE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(EFFECT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUTTON_INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT_PIN), handleButtons, FALLING);
}