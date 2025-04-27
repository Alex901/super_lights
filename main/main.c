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
#include "rgb_led_control.h"  // For RGB LED control
#include "ir_control.h"      // For IR control
#include "us_control.h"      // For ultrasonic sensor control
#include "speaker_control.h"  // For speaker control
#include "memory_control.h"   // For memory control 

void app_main(void)
{
    // Remember that these two are redundant
    gpio_init();
    power_control_init();

    memory_control_init(); 
    display_init();
    auto_turn_off_init(); 
    settings_init(); 
    speaker_init(); 
    us_sensor_init(); 
    rgb_led_control_init(); 
    ir_sensor_init(); 


    // This is a bit silly and should not be here -- lol
    display_render("  Super_Lights", "    V 0.2.8    ");
    vTaskDelay(pdMS_TO_TICKS(500)); // Display for 2 seconds
    display_loading_animation("Loading awesome");

    // Tasks
    xTaskCreate(memory_control_task, "MemoryControlTask", 4096, NULL, 5, NULL); // Create the memory control task

    vTaskDelay(pdMS_TO_TICKS(500));

    menu_init(); // Initialize the menu system

    while (1)
    {
        extern bool is_in_special_mode;
        extern bool is_in_special_mode_lr;
        // Check if the power button is pressed
        if (gpio_get_button_state(POWER_BUTTON_GPIO) == 0)
        {
            memory_print_all_events();

            // Wait for the button to be released
            while (gpio_get_button_state(POWER_BUTTON_GPIO) == 0)
                vTaskDelay(pdMS_TO_TICKS(100)); // Debounce delay
        }

        // If in special mode, skip the rest of the loop
        if (!is_in_special_mode || !is_in_special_mode_lr)
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
        us_sensor_control(); // Check for ultrasonic sensor activity
        ir_sensor_control(); // Check for IR sensor activity
        rgb_led_control_update();
        speaker_update(); 
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}