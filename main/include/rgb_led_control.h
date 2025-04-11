#ifndef RGB_LED_CONTROL_H
#define RGB_LED_CONTROL_H

#include "settings_control.h"

// Initialize the RGB LED control
void rgb_led_control_init(void);

// Update the RGB LED state based on settings
void rgb_led_control_update(void);

// Turn off the RGB LEDs
void rgb_led_control_turn_off(void);

#endif // RGB_LED_CONTROL_H