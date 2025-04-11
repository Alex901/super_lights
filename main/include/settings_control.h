#ifndef SETTINGS_CONTROL_H
#define SETTINGS_CONTROL_H

// Enum for settings
typedef enum {
    SETTING_SOUND,
    SETTING_BRIGHTNESS,
    SETTING_COLOR,
    SETTING_SENSITIVITY_IR,
    SETTING_SENSITIVITY_UR,
    SETTING_MODE,
    SETTING_TIMING_IR,
    SETTING_TIMING_UR,
    SETTING_COUNT,
    SETTING_LIGHT,
    SETTING_LIGHT_AUTO_TURN_OFF
} SettingKey;

// Structure for colors
typedef struct {
    const char *name; // Name of the color
    int r; // Red component (0-255)
    int g; // Green component (0-255)
    int b; // Blue component (0-255)
} Color;

// Settings structure
typedef struct {
    int sound_on; // Sound: 1 = On, 0 = Off
    int brightness; // Brightness: 0-100%
    int selected_color; // Index of the selected color
    int sensitivity_ir; // IR sensitivity: 0-100%
    int sensitivity_ur; // UR sensitivity: 0-100%
    int mode; // Mode: 0 = Manual, 1 = IR, 2 = US, 3 = IR+US
    int timing_ir; // IR timing: 0-100%
    int timing_ur; // UR timing: 0-100%
    int light; // Light: 1 = On, 0 = Off
    int light_auto_turn_off; // Auto turn off: 0 = off, 1+ = seconds until the light turns off automatically
} Settings;

// Initialize settings with default values
void settings_init(void);

// Get a pointer to the settings structure
Settings *settings_get(void);

// Fetch the name of a setting by key
const char *settings_get_name(SettingKey key);

// Fetch the value of a setting by key as a string
const char *settings_get_value(SettingKey key);

// Fetch the RGB values of the selected color
Color settings_get_color(void);

// Update a setting by key
void settings_update(SettingKey key, int value);

// Reset all settings to default values
void settings_reset(void);

// Print all settings to the console
void settings_print_all(void);

#endif // SETTINGS_CONTROL_H