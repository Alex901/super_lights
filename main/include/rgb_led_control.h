#ifndef RGB_LED_CONTROL_H
#define RGB_LED_CONTROL_H

#include "settings_control.h"

// Initialize the RGB LED control
void rgb_led_control_init(void);

// Update the RGB LED state based on settings
void rgb_led_control_update(void);

// Turn off the RGB LEDs
void rgb_led_control_turn_off(void);

// Task function for RGB LED control
void rgb_led_task(void *pvParameters);

#endif // RGB_LED_CONTROL_H