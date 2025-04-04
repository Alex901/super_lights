#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "include/gpio_control.h"
#include "include/button_control.h"
#include "include/power_control.h"

void app_main(void)
{
    // Initialize GPIOs
    gpio_init();

    // Initialize power control
    power_control_init();

    while (1)
    {
        // Check if the button is pressed
        if (button_is_pressed())
        {
            // Toggle power state
            power_control_toggle();
        }

        // Small delay to avoid busy looping
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}