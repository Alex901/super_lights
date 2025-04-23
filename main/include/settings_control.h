#ifndef SETTINGS_CONTROL_H
#define SETTINGS_CONTROL_H

// Enum for settings
typedef enum {
    SETTING_BRIGHTNESS,
    SETTING_COLOR,
    SETTING_IR,
    SETTING_US,
    SETTING_SENSITIVITY_IR,
    SETTING_SENSITIVITY_UR,
    SETTING_TIMING_IR,
    SETTING_TIMING_UR,
    SETTING_LIGHT,
    SETTING_LIGHT_AUTO_TURN_OFF,
    SETTING_SOUND,
    SETTING_VOLUME,
    SETTING_SELECTED_SIGNAL,
    SETTING_COUNT,
} SettingKey;

// Structure for colors
typedef struct {
    const char *name; // Name of the color
    int r; // Red component (0-255)
    int g; // Green component (0-255)
    int b; // Blue component (0-255)
} Color;

typedef struct {
    const char *name;       // Name of the signal
    struct {
        int frequency;      // Frequency of the tone (in Hz)
        int duration;       // Duration of the tone (in ms)
    } tones[10];            // Array of tones (up to 10 tones per signal)
    int tone_count;         // Number of tones in the signal
} Signal;

// Settings structure
typedef struct {
    int brightness; // Brightness: 0-100%
    int selected_color; // Index of the selected color
    int sensitivity_ir; // IR sensitivity: 0-100%
    int sensitivity_ur; // UR sensitivity: 0-100%
    int timing_ir; // IR timing: 0-100%
    int timing_ur; // UR timing: 0-100%
    int light; // Light: 1 = On, 0 = Off
    int light_auto_turn_off; // Auto turn off: 0 = off, 1+ = seconds until the light turns off automatically
    int ir; // IR: 1 = On, 0 = Off
    int us; // US: 1 = On, 0 = Off
    int sound_on; // Sound: 1 = On, 0 = Off
    int volume; // Volume: 0-100%
    int selected_signal; // Choose signal to play
} Settings;

// Initialize settings with default values
void settings_init(void);

// Get a pointer to the settings structure
Settings *settings_get(void);

// Get the selected signal to play
const Signal *get_selected_signal(void);

const char **settings_get_color_names(void);

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

// Initialize the auto turn-off timer
void auto_turn_off_init(void);


#endif // SETTINGS_CONTROL_H