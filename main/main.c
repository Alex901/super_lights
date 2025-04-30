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
#include "ir_control.h"       // For IR control
#include "us_control.h"       // For ultrasonic sensor control
#include "speaker_control.h"  // For speaker control
#include "memory_control.h"   // For memory control
#include "esp_sleep.h"

void app_main(void)
{
    // Check the wake-up reason
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_GPIO:
        printf("Woke up from GPIO wake-up\n");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        printf("Woke up from timer wake-up\n");
        break;
    default:
        printf("Power-on or reset\n");
        break;
    }

    // Remember that these two are redundant
    gpio_init();
    power_control_init();

    // Initialize components
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
    xTaskCreate(us_sensor_task, "UltrasonicSensorTask", 4096, NULL, 5, NULL);   // Create the ultrasonic sensor task
    xTaskCreate(rgb_led_task, "RGBLEDTask", 4096, NULL, 5, NULL);               // Create the RGB LED task
    xTaskCreate(ir_sensor_task, "IRSensorTask", 4096, NULL, 5, NULL);           // Create the IR sensor task
    xTaskCreate(speaker_task, "SpeakerTask", 4096, NULL, 5, NULL);

    vTaskDelay(pdMS_TO_TICKS(200));

    menu_init(); // Initialize the menu system

    int inactivity_counter = 0; // Counter for inactivity

    while (1)
    {
        extern bool is_in_special_mode;
        extern bool is_in_special_mode_lr;

        // Check if the power button is pressed
        if (gpio_get_button_state(POWER_BUTTON_GPIO) == 0)
        {
            settings_print_all();

            // Wait for the button to be released
            while (gpio_get_button_state(POWER_BUTTON_GPIO) == 0)
                vTaskDelay(pdMS_TO_TICKS(100)); // Debounce delay

            inactivity_counter = 0; // Reset inactivity counter
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
                    ;                   // Wait for release
                inactivity_counter = 0; // Reset inactivity counter
            }

            // Check if the back button is pressed
            if (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
            {
                printf("Back button has been pressed\n");
                menu_back();
                while (gpio_get_button_state(BACK_BUTTON_GPIO) == 0)
                    ;                   // Wait for release
                inactivity_counter = 0; // Reset inactivity counter
            }

            // Check if the up button is pressed
            if (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
            {
                printf("Up button has been pressed\n");
                menu_scroll_up();
                while (gpio_get_button_state(UP_BUTTON_GPIO) == 0)
                    ;                   // Wait for release
                inactivity_counter = 0; // Reset inactivity counter
            }

            // Check if the down button is pressed
            if (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
            {
                printf("Down button has been pressed\n");
                menu_scroll_down();
                while (gpio_get_button_state(DOWN_BUTTON_GPIO) == 0)
                    ;                   // Wait for release
                inactivity_counter = 0; // Reset inactivity counter
            }
        }

        // Increment inactivity counter
        inactivity_counter++;

        // Check if inactivity exceeds 1 minute (600 iterations of 100ms)
        if (inactivity_counter >= 600)
        {
            printf("No activity detected for 1 minute, entering deep sleep\n");

            // Display deep sleep message
            display_render("Press enter", "to wake up");

            // Configure GPIOs as wake-up sources
            esp_sleep_enable_ext1_wakeup((1ULL << ENTER_BUTTON_GPIO),
                                         ESP_EXT1_WAKEUP_ALL_LOW);

            // Enter deep sleep
            esp_deep_sleep_start();
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100ms
    }
}