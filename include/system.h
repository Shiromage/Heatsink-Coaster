// system.h
// Chase Baker 2022

#ifndef _SYSTEM_H
#define _SYSTEM_H

#define PIXEL_COUNT 24

#define PIXEL_DATA_PIN  12
#define POWER_SUPPLY_DETECT_PIN A1
#define BUTTON_INTERRUPT_PIN    2
#define COLOR_CYCLE_BUTTON_PIN  4
#define EFFECT_BUTTON_PIN       5
#define BRIGHTNESS_BUTTON_PIN   6

/*
 * The Heatsink Coaster features a ring of LEDs. The first LED on the data line
 * is not quite aligned with either x or y. If one were to look at the front of
 * the device, they will observe that the first LED (index 0) is around the 3
 * to 4 o'clock position, or 352.5 degrees from the observer's perspective.
 * Angular position increases in the counter-clockwise direction. Below are two
 * functions that will help convert LED to degrees, and vice versa. These
 * macros expect input that is within a single revolution ((0, 360] degrees and
 * [0, PIXEL_COUNT-1] LED indices).
 */

//Examples:
//Your 12 'o clock while looking at the front of the device is the system's ~270 degree position
//Your 3 o'clock is the device's ~355 degree position, LED 23
//Your 6 o'clock is the device's ~90 degree position

//These macros have NOT been tested yet.

//Converts degrees into an LED on the ring
#define DEG_TO_LED(_deg)    ((360.0 - _deg) / (360.0 / PIXEL_COUNT))

//Converts an LED index into its angular position
#define LED_TO_DEG(_led)    (360.0 - (360.0 / PIXEL_COUNT) * (_led + 0.5))

#endif // _SYSTEM_H
