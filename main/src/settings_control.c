#include "settings_control.h"
#include <stdio.h>
#include <string.h>

// Predefined colors with names and RGB values
static const Color colors[] = {
    {"Red", 255, 0, 0},
    {"Green", 0, 255, 0},
    {"Blue", 0, 0, 255},
    {"White", 255, 255, 255},
    {"Yellow", 255, 255, 0},
    {"Cyan", 0, 255, 255},
    {"Magenta", 255, 0, 255}
};

// Predefined modes
static const char *mode_names[] = {"Manual", "IR", "US", "IR+US"};

// Settings instance
static Settings settings;

// Initialize settings with default values
void settings_init(void)
{
    settings.sound_on = 0; // Sound OFF
    settings.brightness = 50; // 50% brightness
    settings.selected_color = 0; // Default to "Red"
    settings.sensitivity_ir = 50; // 50% IR sensitivity
    settings.sensitivity_ur = 50; // 50% UR sensitivity
    settings.mode = 0; // Default to "Manual"
    settings.timing_ir = 50; // 50% IR timing
    settings.timing_ur = 50; // 50% UR timing
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
    case SETTING_SOUND: return "Sound";
    case SETTING_BRIGHTNESS: return "Brightness";
    case SETTING_COLOR: return "Color";
    case SETTING_SENSITIVITY_IR: return "IR Sensitivity";
    case SETTING_SENSITIVITY_UR: return "UR Sensitivity";
    case SETTING_MODE: return "Mode";
    case SETTING_TIMING_IR: return "IR Timing";
    case SETTING_TIMING_UR: return "UR Timing";
    default: return "Unknown";
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
    case SETTING_MODE:
        return mode_names[settings.mode];
    case SETTING_TIMING_IR:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.timing_ir);
        return value_str;
    case SETTING_TIMING_UR:
        snprintf(value_str, sizeof(value_str), "%d%%", settings.timing_ur);
        return value_str;
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
    case SETTING_MODE:
        if (value >= 0 && value < (int)(sizeof(mode_names) / sizeof(mode_names[0])))
            settings.mode = value;
        break;
    case SETTING_TIMING_IR:
        if (value >= 0 && value <= 100)
            settings.timing_ir = value;
        break;
    case SETTING_TIMING_UR:
        if (value >= 0 && value <= 100)
            settings.timing_ur = value;
        break;
    default:
        break;
    }
}

// Reset all settings to default values
void settings_reset(void)
{
    settings_init();
}