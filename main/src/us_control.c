#include "us_control.h"
#include "settings_control.h"
#include "rgb_led_control.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include <stdio.h>
#include "menu_control.h"
#include "rgb_led_control.h"
#include "esp_task_wdt.h"

#define TRIG_PIN GPIO_NUM_26
#define ECHO_PIN GPIO_NUM_5
#define SOUND_SPEED_CM_PER_US 0.0343 // Speed of sound in cm/µs

// Initialize the ultrasonic sensor
void us_sensor_init(void)
{
    // Configure the TRIG pin as output
    gpio_config_t trig_config = {
        .pin_bit_mask = (1ULL << TRIG_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&trig_config);

    // Configure the ECHO pin as input
    gpio_config_t echo_config = {
        .pin_bit_mask = (1ULL << ECHO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&echo_config);

    printf("Ultrasonic sensor initialized (TRIG: GPIO %d, ECHO: GPIO %d)\n", TRIG_PIN, ECHO_PIN);
}

// Measure distance using the ultrasonic sensor
float us_sensor_get_distance(void)
{
    // Send a 10µs pulse on the TRIG pin
    gpio_set_level(TRIG_PIN, 1);
    esp_rom_delay_us(10); // Use esp_rom_delay_us instead of ets_delay_us
    gpio_set_level(TRIG_PIN, 0);

    // Wait for the ECHO pin to go HIGH (start of echo)
    int64_t start_time = esp_timer_get_time();
    int64_t timeout = start_time + 24000; // 24ms timeout for ~4m max range
    while (gpio_get_level(ECHO_PIN) == 0)
    {
        if (esp_timer_get_time() > timeout)
        {
            printf("Timeout waiting for ECHO to go HIGH\n");
            return -1; // Return error
        }
    }

    // Measure the duration of the HIGH signal (end of echo)
    start_time = esp_timer_get_time();
    timeout = start_time + 24000; // 24ms timeout
    while (gpio_get_level(ECHO_PIN) == 1)
    {
        if (esp_timer_get_time() > timeout)
        {
            printf("Timeout waiting for ECHO to go LOW\n");
            return -1; // Return error
        }
    }

    // Calculate the duration in microseconds
    int64_t end_time = esp_timer_get_time();
    int64_t duration_us = end_time - start_time;

    // Calculate the distance in cm
    float distance_cm = (duration_us * SOUND_SPEED_CM_PER_US) / 2.0;

    return distance_cm;
}

// Adjust brigtness based on distance, fun little test. Setting might be implemented
// if there is time for it later.
void us_sensor_set_brightness(float distance)
{
    Settings *settings = settings_get();

    // Check if the distance is valid
    if (distance < 0)
    {
        printf("Invalid distance, skipping brightness adjustment\n");
        return;
    }

    // Define the maximum distance for brightness scaling
    int max_distance = 100; // Maximum distance in cm for full brightness scaling

    // If the distance is greater than max_distance, do nothing
    if (distance > max_distance)
    {
        printf("Distance: %.2f cm, out of range (>%d cm), no brightness adjustment\n", distance, max_distance);
        return;
    }

    // Map the distance to brightness (closer = brighter)
    int brightness = (int)((1.0 - (distance / max_distance)) * 100); // Scale brightness (0-100%)
    if (brightness < 0)
        brightness = 0; // Ensure brightness is not negative

    // Update the brightness setting
    settings->brightness = brightness;
    rgb_led_control_update(); // Apply the brightness to the LEDs

    printf("Distance: %.2f cm, Brightness set to: %d%%\n", distance, brightness);
}

// Control the ultrasonic sensor
void us_sensor_control(void)
{
    static int light_turned_off = 0; // Track if the light was turned off
    Settings *settings = settings_get();

    // Check if the ultrasonic sensor is enabled
    if (settings->us == 0)
    {
        return; // US is disabled, do nothing
    }

    // Measure the distance
    float distance = us_sensor_get_distance();
    if (distance < 0)
    {
        printf("Failed to measure distance\n");
        return; // Skip further processing if measurement failed
    }

    // Set the brightness based on the distance
    // us_sensor_set_brightness(distance);

    //printf("Measured distance: %.2f cm\n", distance);

    //Check if the distance is greater than the sensitivity threshold and the light is ON
    if (distance < settings->sensitivity_ur && settings->light == 1)
    {
        printf("Distance > sensitivity, turning off light\n");
        settings->light = 0;      // Turn off the light
        rgb_led_control_update(); // Update the LED state
        printf("Light turned off due to ultrasonic sensor (distance > sensitivity).\n");
        light_turned_off = 1;
        menu_render();
    }
    else if (distance <= settings->sensitivity_ur && light_turned_off)
    {
        light_turned_off = 0; // Reset the flag when the object moves closer
    }
}

void us_sensor_task(void *pvParameters)
{
    esp_task_wdt_add(NULL); // Register the task with the watchdog
    while (1)
    {
        Settings *settings = settings_get();

        if (settings->us == 0)
        {
            printf("Ultrasonic sensor is disabled, entering sleep mode\n");
            esp_task_wdt_reset(); // Feed the watchdog
            vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 second
            continue;
        }

        us_sensor_control(); // Control the ultrasonic sensor
        esp_task_wdt_reset(); // Feed the watchdog
        vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100ms
    }
    esp_task_wdt_delete(NULL); // Unregister the task (if it ever exits)
}
