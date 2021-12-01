#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_COUNT 24
#define PIXEL_DATA_PIN  12

Adafruit_NeoPixel Stage(PIXEL_COUNT, PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    //Determine if power from 9V is available. If not, do not power pixels.

    Stage.begin();
}

void loop()
{

}

void setupButton()
{
  pinMode(PIN_BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), changeDraw, FALLING);
}