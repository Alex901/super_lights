#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include "driver/gpio.h"

// GPIO pin definitions
#define POWER_LED_GPIO GPIO_NUM_4 
#define POWER_BUTTON_GPIO GPIO_NUM_12 // Used for debugging
#define ENTER_BUTTON_GPIO GPIO_NUM_15
#define BACK_BUTTON_GPIO GPIO_NUM_18
#define UP_BUTTON_GPIO GPIO_NUM_19
#define DOWN_BUTTON_GPIO GPIO_NUM_21

void gpio_init(void);
void gpio_set_led(int state);
int gpio_get_button_state(int gpio_num);

#endif // GPIO_CONTROL_H