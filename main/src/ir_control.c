#include "ir_control.h"
#include "settings_control.h"
#include "rgb_led_control.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

// GPIO pin for the IR sensor
#define IR_SENSOR_GPIO GPIO_NUM_27

// Sensitivity placeholder (future implementation)
static int ir_sensitivity = 50; // Default sensitivity (0-100%)

// Initialize the IR sensor
void ir_sensor_init(void)
{
    // Configure the GPIO pin for the IR sensor as input
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << IR_SENSOR_GPIO), // Select GPIO 27
        .mode = GPIO_MODE_INPUT,                  // Set as input
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE, // Enable pull-down resistor
        .intr_type = GPIO_INTR_DISABLE // No interrupts for now
    };
    gpio_config(&io_conf);

    printf("IR sensor initialized on GPIO %d\n", IR_SENSOR_GPIO);
}

// Control the IR sensor
void ir_sensor_control(void)
{
    static int previous_ir_state = -1; // Initialize to an invalid state to detect the first run
    Settings *settings = settings_get();

    // Check if IR is enabled in the settings
    if (settings->ir == 0)
    {
        return; // IR is disabled, do nothing
    }

    // Read the current state of the IR sensor
    int current_ir_state = gpio_get_level(IR_SENSOR_GPIO);

    // Log the current and previous states
    printf("IR sensor state: current = %d, previous = %d\n", current_ir_state, previous_ir_state);

    // Check for state changes
    if (current_ir_state != previous_ir_state)
    {
        printf("IR sensor state changed: %d -> %d\n", previous_ir_state, current_ir_state);
        previous_ir_state = current_ir_state; // Update the previous state
    }

    // If the sensor is tripped (motion detected)
    if (current_ir_state == 1)
    {
        // Debounce: Check if the signal remains stable for 50ms
        vTaskDelay(pdMS_TO_TICKS(50)); // Wait 50ms
        if (gpio_get_level(IR_SENSOR_GPIO) == 1)
        {
            // Turn on the light
            settings->light = 1;
            rgb_led_control_update(); // Update the LED state
            printf("Motion detected! Light turned on.\n");
        }
    }
}

// Set the sensitivity of the IR sensor (boilerplate for future implementation)
void ir_sensor_set_sensitivity(int sensitivity)
{
    if (sensitivity < 0)
        sensitivity = 0;
    if (sensitivity > 100)
        sensitivity = 100;

    ir_sensitivity = sensitivity;
    printf("IR sensor sensitivity set to %d%%\n", ir_sensitivity);

    // Future implementation: Adjust the IR sensor's behavior based on sensitivity
}