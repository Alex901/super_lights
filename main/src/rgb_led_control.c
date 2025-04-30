#include "rgb_led_control.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "settings_control.h"
#include <stdio.h>
#include "esp_task_wdt.h"

// GPIO pin for the RGB LED signal
#define RGB_LED_GPIO GPIO_NUM_13
#define LED_COUNT 8 // Number of LEDs in the module

static led_strip_handle_t led_strip;

// Initialize the RGB LED control
void rgb_led_control_init(void)
{
    // Configure the LED strip
    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_LED_GPIO,
        .max_leds = LED_COUNT,                    // Number of LEDs in the strip
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // WS2812 uses GRB format
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz resolution
    };

    // Initialize the LED strip
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    // Clear the LED strip
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
}

// Update the RGB LED state based on settings
void rgb_led_control_update(void)
{
    Settings *settings = settings_get();

    // printf("checking settings\n");
    if (settings->light == 0)
    {
        printf("Sup bitchess #2\n");
        rgb_led_control_turn_off();
        return;
    }


    // Get the brightness and color settings
    int brightness = settings->brightness; // 0-100%
    Color color = settings_get_color();

    // Scale the RGB values based on brightness
    uint8_t red = (color.r * brightness) / 100;
    uint8_t green = (color.g * brightness) / 100;
    uint8_t blue = (color.b * brightness) / 100;

    // Set the color for all LEDs
    for (int i = 0; i < LED_COUNT; i++)
    {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
    }

    // Refresh the LED strip to apply changes
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}

// Turn off the RGB LEDs, and no.. This is not proper error handling so shush!
void rgb_led_control_turn_off(void)
{
    if (led_strip == NULL)
    {
        printf("Error: LED strip not initialized\n");
        rgb_led_control_init(); // Reinitialize the LED strip if not initialized
        return;
    }

    esp_err_t err = led_strip_clear(led_strip); // Attempt to clear the LED strip
    if (err != ESP_OK)
    {
        printf("Error turning off LEDs: %s\n", esp_err_to_name(err));
        return;
    }
    else
    {
        printf("LEDs turned off successfully\n");
    }
}

void rgb_led_task(void *pvParameters)
{
    esp_task_wdt_add(NULL); // Register the task with the watchdog

    Settings *settings = settings_get();
    static int previous_light_state = -1; // Track the previous light state
    static int previous_brightness = -1; // Track the previous brightness
    static int previous_color = -1;      // Track the previous color

    while (1)
    {
        // Check if the light is off
        if (settings->light == 0)
        {
            // Turn off the LEDs if the light state has changed
            if (previous_light_state != 0)
            {
                printf("Light is off, turning off LEDs\n");
                rgb_led_control_turn_off();
                previous_light_state = 0; // Update the previous state
            }

            // printf("Light is off, entering sleep mode\n");
            esp_task_wdt_reset(); // Feed the watchdog
            vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 second
            continue;
        }

        // Check for changes in relevant settings
        if (settings->light != previous_light_state ||
            settings->brightness != previous_brightness ||
            settings->selected_color != previous_color)
        {
            printf("Sup bitchess!");
            // Call the update function if any relevant setting has changed
            rgb_led_control_update();

            // Update the previous state
            previous_light_state = settings->light;
            previous_brightness = settings->brightness;
            previous_color = settings->selected_color;
        }

        // Feed the watchdog
        esp_task_wdt_reset();

        // Delay to avoid busy looping
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }

    esp_task_wdt_delete(NULL); // Unregister the task (if it ever exits)
}