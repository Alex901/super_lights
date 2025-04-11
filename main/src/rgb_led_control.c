#include "rgb_led_control.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// GPIO pin for the RGB LED signal
#define RGB_LED_GPIO GPIO_NUM_27

// LEDC timer and channel configuration
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_RED LEDC_CHANNEL_0
#define LEDC_CHANNEL_GREEN LEDC_CHANNEL_1
#define LEDC_CHANNEL_BLUE LEDC_CHANNEL_2
#define LEDC_DUTY_RES LEDC_TIMER_8_BIT // 8-bit resolution (0-255)
#define LEDC_FREQUENCY 5000            // 5 kHz PWM frequency

// Initialize the RGB LED control
void rgb_led_control_init(void)
{
    // Configure the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    // Configure the LEDC channels for Red, Green, and Blue
    ledc_channel_config_t ledc_channel_red = {
        .gpio_num = RGB_LED_GPIO,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_RED,
        .timer_sel = LEDC_TIMER,
        .duty = 0, // Start with LED off
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel_red);

    // Repeat for Green and Blue channels
    ledc_channel_config_t ledc_channel_green = ledc_channel_red;
    ledc_channel_config_t ledc_channel_blue = ledc_channel_red;

    ledc_channel_green.channel = LEDC_CHANNEL_GREEN;
    ledc_channel_blue.channel = LEDC_CHANNEL_BLUE;

    ledc_channel_config(&ledc_channel_green);
    ledc_channel_config(&ledc_channel_blue);
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
    int red = (color.r * brightness) / 100;
    int green = (color.g * brightness) / 100;
    int blue = (color.b * brightness) / 100;

    // Update the LEDC duty cycle for each color channel
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_RED, red);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_RED);

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_GREEN, green);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_GREEN);

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_BLUE, blue);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_BLUE);

    // Handle auto turn-off if enabled
    if (settings->light_auto_turn_off > 0)
    {
        vTaskDelay(pdMS_TO_TICKS(settings->light_auto_turn_off * 1000));
        rgb_led_control_turn_off();
    }
}

// Turn off the RGB LEDs
void rgb_led_control_turn_off(void)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_RED, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_RED);

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_GREEN, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_GREEN);

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_BLUE, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_BLUE);
}