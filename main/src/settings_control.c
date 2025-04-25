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
    {"Magenta", 255, 0, 255}};

static const Signal signals[] = {
    {"Beep", {{1000, 200}}, 1},                                        // A single beep at 1 kHz for 200 ms
    {"Double Beep", {{1000, 200}, {1000, 200}}, 2},                    // Two short beeps
    {"Chime", {{500, 500}, {700, 500}, {900, 500}}, 3},                // A chime with rising tones
    {"Alarm", {{2000, 500}, {1500, 500}, {2000, 500}}, 3},             // Alternating alarm tones
    {"Melody", {{800, 300}, {1000, 300}, {1200, 300}, {1000, 300}}, 4} // A simple melody
};

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
    settings.brightness = 50;         // 10% brightness
    settings.selected_color = 5;      // Default to "Red"
    settings.sensitivity_ir = 90;     // 90% IR sensitivity
    settings.sensitivity_ur = 50;     // 50% UR sensitivity
    settings.timing_ir = 50;          // 50% IR timing
    settings.timing_ur = 50;          // 50% UR timing
    settings.light = 0;               // Light OFF
    settings.light_auto_turn_off = 0; // No auto turn off
    settings.ir = 1;                  // IR ON
    settings.us = 1;                  // US ON
    settings.sound_on = 1;            // Sound OFF
    settings.volume = 100;            // Volume 50%
    settings.selected_signal = 2;     // Default signal
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
    case SETTING_VOLUME:
        return "Volume";
    case SETTING_SELECTED_SIGNAL:
        return "Signal";

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
    case SETTING_BRIGHTNESS:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.brightness);
        return value_str;
    case SETTING_COLOR:
        return colors[settings.selected_color].name;
    case SETTING_SENSITIVITY_IR:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.sensitivity_ir);
        return value_str;
    case SETTING_SENSITIVITY_UR:
        snprintf(value_str, sizeof(value_str), "%d cm", settings.sensitivity_ur);
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
    case SETTING_SOUND:
        return settings.sound_on ? "On" : "Off";
    case SETTING_VOLUME:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.volume);
        return value_str;
    case SETTING_SELECTED_SIGNAL:
        return signals[settings.selected_signal].name;
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
    case SETTING_BRIGHTNESS:
        if (value >= 0 && value <= 100)
            settings.brightness = value;
        break;
    case SETTING_COLOR:
        if (value >= 0 && value < (int)(sizeof(colors) / sizeof(colors[0])))
            settings.selected_color = value;
        break;
    case SETTING_SENSITIVITY_IR:
        if (value >= 1 && value <= 100)
            settings.sensitivity_ir = value;
        break;
    case SETTING_SENSITIVITY_UR:
        if (value >= 1 && value <= 100)
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
    case SETTING_SOUND:
        settings.sound_on = value ? 1 : 0;
        break;
    case SETTING_VOLUME:
        if (value >= 0 && value <= 100)
            settings.volume = value;
        break;
    case SETTING_SELECTED_SIGNAL:
        if (value >= 0 && value < (int)(sizeof(signals) / sizeof(signals[0])))
            settings.selected_signal = value;
        break;
    default:
        break;
    }
}

// get all the color names
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

const char **settings_get_signal_names(void)
{
    static const char *signal_names[sizeof(signals) / sizeof(signals[0]) + 1]; // +1 for NULL terminator
    static int initialized = 0;

    if (!initialized)
    {
        for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); i++)
        {
            signal_names[i] = signals[i].name; // Fetch the name from the signals array
        }
        signal_names[sizeof(signals) / sizeof(signals[0])] = NULL; // Add NULL terminator
        initialized = 1;
    }

    return signal_names;
}

// Reset all settings to default values
void settings_reset(void)
{
    settings_init();
}

void settings_print_all(void)
{
    printf("Current Settings:\n");

    for (SettingKey key = 0; key < SETTING_COUNT; key++) // Iterate from 0 to SETTING_COUNT - 1
    {
        printf("%s: %s\n", settings_get_name(key), settings_get_value(key));
    }
}

const Signal *get_selected_signal(void)
{
    return &signals[settings.selected_signal];
}

// Should be its own file, but CBA to do that right now

// Forward declaration of function
void auto_turn_off_task(void *pvParameters);

// Initialize the auto turn-off system
void auto_turn_off_init(void)
{
    // Create the timer (one-shot timer)
    auto_turn_off_timer = xTimerCreate(
        "AutoTurnOffTimer",    // Timer name
        pdMS_TO_TICKS(1000),   // Dummy period (will be updated dynamically)
        pdFALSE,               // One-shot timer
        (void *)0,             // Timer ID (not used)
        auto_turn_off_callback // Callback function
    );

    if (auto_turn_off_timer == NULL)
    {
        printf("Failed to create auto turn-off timer.\n");
    }

    // Create the background task
    xTaskCreate(auto_turn_off_task, "AutoTurnOffTask", 2048, NULL, 5, NULL);
}

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

// Background task to monitor the light setting
void auto_turn_off_task(void *pvParameters)
{
    static int has_started = 0; // Tracks whether the timer has started

    while (1)
    {
        Settings *settings = settings_get();

        // Check if the light is on and the timer hasn't started
        if (settings->light == 1 && has_started == 0)
        {
            auto_turn_off_start(); // Start the timer
            has_started = 1;       // Mark the timer as started
        }
        // Check if the light is off and the timer was running
        else if (settings->light == 0 && has_started == 1)
        {
            if (xTimerIsTimerActive(auto_turn_off_timer))
            {
                xTimerStop(auto_turn_off_timer, 0); // Stop the timer
                printf("Auto turn-off timer stopped because the light was turned off.\n");
            }
            has_started = 0; // Reset the flag
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms
    }
}

void play_signal(const Signal *signal)
{
    for (int i = 0; i < signal->tone_count; i++)
    {
        int frequency = signal->tones[i].frequency;
        int duration = signal->tones[i].duration;

        // Play the tone (replace with your hardware-specific function)
        printf("Playing tone: Frequency = %d Hz, Duration = %d ms\n", frequency, duration);
        // Example: audio_play_tone(frequency, duration);

        // Delay for the duration of the tone
        vTaskDelay(pdMS_TO_TICKS(duration));
    }
}