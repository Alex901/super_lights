#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_control.h"       // For GPIO initialization and button state reading
#include "button_control.h"     // For button handling logic
#include "power_control.h"      // For power control functionality
#include "driver/gpio.h"        // For GPIO_NUM_x constants

void app_main(void)
{
    // Initialize GPIOs
    gpio_init();

    // Initialize power control
    power_control_init();

    while (1)
    {
        // Check if the power button is pressed
        if (gpio_get_button_state(POWER_BUTTON_GPIO) == 0)
        {
            printf("Power button has been pressed\n");
            power_control_toggle(); // Toggle power state
            while (gpio_get_button_state(POWER_BUTTON_GPIO) == 0); // Wait for release
        }

        // Check if the enter button is pressed
        if (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
        {
            printf("Enter button has been pressed\n");
            while (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0); // Wait for release
        }

        // Check if the back button is pressed
        if (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
        {
            printf("Back button has been pressed\n");
            while (gpio_get_button_state(BACK_BUTTON_GPIO) == 0); // Wait for release
        }

        // Check if the up button is pressed
        if (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
        {
            printf("Up button has been pressed\n");
            while (gpio_get_button_state(UP_BUTTON_GPIO) == 0); // Wait for release
        }

        // Check if the down button is pressed
        if (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
        {
            printf("Down button has been pressed\n");
            while (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0); // Wait for release
        }

        // Small delay to avoid busy looping
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}