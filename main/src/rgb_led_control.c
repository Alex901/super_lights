#include "rgb_led_control.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "settings_control.h"
#include <stdio.h>

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

    // If the light is off, turn off the LEDs
    if (!settings->light)
    {
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

// Turn off the RGB LEDs
void rgb_led_control_turn_off(void)
{
    ESP_ERROR_CHECK(led_strip_clear(led_strip)); // Turn off all LEDs
}