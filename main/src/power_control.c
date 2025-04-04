#include "power_control.h"
#include "gpio_control.h"
#include <stdio.h>

static int power_state = 0; // 0 = OFF, 1 = ON

void power_control_init(void)
{
    gpio_set_led(power_state); // Initialize LED to OFF
}

void power_control_toggle(void)
{
    power_state = !power_state; // Toggle power state
    gpio_set_led(power_state);  // Update LED state

    // Print power status
    if (power_state)
    {
        printf("Power ON\n");
    }
    else
    {
        printf("Power OFF\n");
    }
}