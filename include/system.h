//system.h

#ifndef _SYSTEM_H
#define _SYSTEM_H

#define PIXEL_COUNT 24

/*
 * The Heatsink Coaster features a ring of LEDs. The first LED on the data line
 * is not quite aligned with either x or y. If one were to look at the front of
 * the device, they will observe that the first LED (index 0) is around the 3
 * to 4 o'clock position, at 352.5 degrees for an angular position that is
 * based from the 3 o'clock position and increases with counter-clockwise
 * motion. Below are two functions that will help convert LED to degrees, and
 * vice versa. These macros expect input that is within a single revolution
 * ((0, 360] degrees and [0, PIXEL_COUNT-1] LED indices).
 */

//Converts degrees into an LED on the ring
#define DEG_TO_LED(_deg)    ((360.0 - _deg) / (360.0 / PIXEL_COUNT))

//Converts an LED index into its angular position
#define LED_TO_DEG(_led)    (360.0 - (360.0 / PIXEL_COUNT) * (_led + 0.5))

#endif // _SYSTEM_H
