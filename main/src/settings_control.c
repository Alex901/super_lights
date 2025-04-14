#include "settings_control.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "rgb_led_control.h"

// Timer handle for auto turn-off
static TimerHandle_t auto_turn_off_timer = NULL;

// Predefined colors with names and RGB values
static const Color colors[] = {
    {"Red", 255, 0, 0},
    {"White", 255, 255, 255}, // Default color
    {"Green", 0, 255, 0},
    {"Blue", 0, 0, 255},
    {"Yellow", 255, 255, 0},
    {"Cyan", 0, 255, 255},
    {"Magenta", 255, 0, 255}
};

// Predefined modes
static const char *mode_names[] = {"Manual", "IR", "US", "IR+US"};

// Settings instance
static Settings settings;

// Callback function for the timer
static void auto_turn_off_callback(TimerHandle_t xTimer)
{
    Settings *settings = settings_get();

    // Turn off the light
    settings->light = 0;
    rgb_led_control_update(); // Update the LED state
    printf("Light turned off due to auto unplug timer.\n");
}

// Initialize settings with default values
void settings_init(void)
{
    settings.sound_on = 0;            // Sound OFF
    settings.brightness = 50;         // 50% brightness
    settings.selected_color = 0;      // Default to "Red"
    settings.sensitivity_ir = 50;     // 50% IR sensitivity
    settings.sensitivity_ur = 50;     // 50% UR sensitivity
    settings.timing_ir = 50;          // 50% IR timing
    settings.timing_ur = 50;          // 50% UR timing
    settings.light = 0;            // Light OFF
    settings.light_auto_turn_off = 0; // No auto turn off
    settings.ir = 1;
    settings.us = 0;
}

// Get a pointer to the settings structure
Settings *settings_get(void)
{
    return &settings;
}

// Fetch the name of a setting by key
const char *settings_get_name(SettingKey key)
{
    switch (key)
    {
    case SETTING_SOUND:
        return "Sound";
    case SETTING_BRIGHTNESS:
        return "Brightness";
    case SETTING_COLOR:
        return "Color";
    case SETTING_SENSITIVITY_IR:
        return "IR Sensitivity";
    case SETTING_SENSITIVITY_UR:
        return "UR Sensitivity";
    case SETTING_TIMING_IR:
        return "IR Timing";
    case SETTING_TIMING_UR:
        return "UR Timing";
    case SETTING_LIGHT:
        return "Light";
    case SETTING_LIGHT_AUTO_TURN_OFF:
        return "Auto unplug";
    case SETTING_IR:
        return "IR";
    case SETTING_US:
        return "US";
    default:
        return "Unknown";
    }
}

// Fetch the value of a setting by key as a string
const char *settings_get_value(SettingKey key)
{
    static char value_str[32]; // Buffer for the value string

    switch (key)
    {
    case SETTING_SOUND:
        return settings.sound_on ? "On" : "Off";
    case SETTING_BRIGHTNESS:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.brightness);
        return value_str;
    case SETTING_COLOR:
        return colors[settings.selected_color].name;
    case SETTING_SENSITIVITY_IR:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.sensitivity_ir);
        return value_str;
    case SETTING_SENSITIVITY_UR:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.sensitivity_ur);
        return value_str;
    case SETTING_TIMING_IR:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.timing_ir);
        return value_str;
    case SETTING_TIMING_UR:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.timing_ur);
        return value_str;
    case SETTING_LIGHT:
        return settings.light ? "On" : "Off";
    case SETTING_LIGHT_AUTO_TURN_OFF: 
        if (settings.light_auto_turn_off == 0)
            return "Off";
        snprintf(value_str, sizeof(value_str), "%d sec", settings.light_auto_turn_off);
        return value_str;
    case SETTING_IR:
        return settings.ir ? "On" : "Off";
    case SETTING_US:
        return settings.us ? "On" : "Off";
    default:
        return "Unknown";
    }
}

// Fetch the RGB values of the selected color
Color settings_get_color(void)
{
    return colors[settings.selected_color];
}

// Update a setting by key
void settings_update(SettingKey key, int value)
{
    switch (key)
    {
    case SETTING_SOUND:
        settings.sound_on = value ? 1 : 0;
        break;
    case SETTING_BRIGHTNESS:
        if (value >= 0 && value <= 100)
            settings.brightness = value;
        break;
    case SETTING_COLOR:
        if (value >= 0 && value < (int)(sizeof(colors) / sizeof(colors[0])))
            settings.selected_color = value;
        break;
    case SETTING_SENSITIVITY_IR:
        if (value >= 0 && value <= 100)
            settings.sensitivity_ir = value;
        break;
    case SETTING_SENSITIVITY_UR:
        if (value >= 0 && value <= 100)
            settings.sensitivity_ur = value;
        break;
    case SETTING_TIMING_IR:
        if (value >= 0 && value <= 100)
            settings.timing_ir = value;
        break;
    case SETTING_TIMING_UR:
        if (value >= 0 && value <= 100)
            settings.timing_ur = value;
        break;
    case SETTING_LIGHT:
        settings.light = value ? 1 : 0;
        break;
          case SETTING_LIGHT_AUTO_TURN_OFF: // New case
        if (value >= 0 && value <= 60)
            settings.light_auto_turn_off = value;
        break;
    case SETTING_IR:
        settings.ir = value ? 1 : 0;
        break;
    case SETTING_US:
        settings.us = value ? 1 : 0;
        break;
    default:
        break;
    }
}

//get all the color names
const char **settings_get_color_names(void)
{
    static const char *color_names[sizeof(colors) / sizeof(colors[0]) + 1]; // +1 for NULL terminator
    static int initialized = 0;

    if (!initialized)
    {
        for (size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); i++)
        {
            color_names[i] = colors[i].name; // Fetch the name from the colors array
        }
        color_names[sizeof(colors) / sizeof(colors[0])] = NULL; // Add NULL terminator
        initialized = 1;
    }

    return color_names;
}

// Reset all settings to default values
void settings_reset(void)
{
    settings_init();
}

void settings_print_all(void)
{
    printf("Current Settings:\n");

    for (SettingKey key = SETTING_SOUND; key <= SETTING_LIGHT_AUTO_TURN_OFF; key++)
    {
        printf("%s: %s\n", settings_get_name(key), settings_get_value(key));
    }
}

// Initialize the auto turn-off timer
void auto_turn_off_init(void)
{
    // Create the timer (one-shot timer)
    auto_turn_off_timer = xTimerCreate(
        "AutoTurnOffTimer",          // Timer name
        pdMS_TO_TICKS(1000),         // Dummy period (will be updated dynamically)
        pdFALSE,                     // One-shot timer
        (void *)0,                   // Timer ID (not used)
        auto_turn_off_callback       // Callback function
    );

    if (auto_turn_off_timer == NULL)
    {
        printf("Failed to create auto turn-off timer.\n");
    }
}

// Start or stop the auto turn-off timer based on the setting
void auto_turn_off_start(void)
{
    Settings *settings = settings_get();

    // Stop the timer if the value is 0
    if (settings->light_auto_turn_off == 0)
    {
        if (xTimerIsTimerActive(auto_turn_off_timer))
        {
            xTimerStop(auto_turn_off_timer, 0);
            printf("Auto turn-off timer stopped.\n");
        }
        return;
    }

    // Start the timer with the specified duration
    TickType_t timer_period = pdMS_TO_TICKS(settings->light_auto_turn_off * 1000);
    xTimerChangePeriod(auto_turn_off_timer, timer_period, 0);
    xTimerStart(auto_turn_off_timer, 0);
    printf("Auto turn-off timer started for %d seconds.\n", settings->light_auto_turn_off);
}

