#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_control.h"     // For GPIO initialization and button state reading
#include "button_control.h"   // For button handling logic
#include "power_control.h"    // For power control functionality
#include "driver/gpio.h"      // For GPIO_NUM_x constants
#include "display_control.h"  // For display control
#include "settings_control.h" // For settings management
#include "menu_control.h"     // For menu control

void app_main(void)
{
    // Initialize GPIOs
    gpio_init();

    // Initialize power control
    power_control_init();

    display_init();

    // Super_Lights screen
    display_render("  Super_Lights", "    V 0.0.4    ");
    vTaskDelay(pdMS_TO_TICKS(2000)); // Display for 2 seconds

    settings_init(); // Initialize settings with default values

    // Loading screen
    display_loading_animation("Loading awesome");

    menu_init(); // Initialize the menu system

    vTaskDelay(pdMS_TO_TICKS(1000));

    while (1)
    {
        extern bool is_in_special_mode;
        // Check if the power button is pressed
        if (gpio_get_button_state(POWER_BUTTON_GPIO) == 0)
        {
            printf("Power button has been pressed\n");
            power_control_toggle(); // Toggle power state
            while (gpio_get_button_state(POWER_BUTTON_GPIO) == 0)
                ; // Wait for release
        }

         // If in special mode, skip the rest of the loop
        if (!is_in_special_mode)
        {
            // Check if the enter button is pressed
            if (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
            {
                printf("Enter button has been pressed\n");
                menu_select();
                while (gpio_get_button_state(ENTER_BUTTON_GPIO) == 0)
                    ; // Wait for release
            }

            // Check if the back button is pressed
            if (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
            {
                printf("Back button has been pressed\n");
                menu_back(); // Go back one step in the menu
                while (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
                    ; // Wait for release
            }

            // Check if the up button is pressed
            if (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
            {
                printf("Up button has been pressed\n");
                menu_scroll_up(); // Scroll up in the menu
                while (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
                    ; // Wait for release
            }

            // Check if the down button is pressed
            if (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
            {
                printf("Down button has been pressed\n");
                menu_scroll_down(); // Scroll down in the menu
                while (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
                    ; // Wait for release
            }
        }

        // Small delay to avoid busy looping
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}