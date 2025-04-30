#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "speaker_control.h"
#include "settings_control.h"
#include "esp_task_wdt.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SPEAKER_DIN_PIN GPIO_NUM_33 // Audio Data (DIN)
#define SPEAKER_BCK_PIN GPIO_NUM_25 // Bit Clock (BCK)
#define SPEAKER_LCK_PIN GPIO_NUM_32 // Left/Right Clock (LCK)

#define SAMPLE_RATE 44100 // Audio sample rate

static i2s_chan_handle_t tx_channel; // Handle for the TX channel
static bool is_i2s_channel_enabled = false; 

// This is black magic to me, so no questions please. 
// But I'm fairly sure that it initiates the I2S driver in a way that it works -- lol.
void speaker_init(void)
{
    is_i2s_channel_enabled = true;
    // Configure the I2S clock using the default macro
    i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE);

    // Configure the I2S slot using the default Philips slot macro
    i2s_std_slot_config_t slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
        I2S_DATA_BIT_WIDTH_16BIT, // Data bit width
        I2S_SLOT_MODE_MONO      // Mono mode
    );

    // Configure the I2S GPIO pins
    i2s_std_gpio_config_t gpio_cfg = {
        .bclk = SPEAKER_BCK_PIN,
        .ws = SPEAKER_LCK_PIN,
        .dout = SPEAKER_DIN_PIN,
        .din = I2S_GPIO_UNUSED,
    };

    // Create the TX channel
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 16,
        .dma_frame_num = 1024,
        .auto_clear = true,
    };

    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_channel, NULL));

    // Initialize the I2S channel with the combined configuration
    i2s_std_config_t std_cfg = {
        .clk_cfg = clk_cfg,
        .slot_cfg = slot_cfg,
        .gpio_cfg = gpio_cfg,
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_channel, &std_cfg));

    // Enable the I2S channel
    ESP_ERROR_CHECK(i2s_channel_enable(tx_channel));

    // printf("Speaker initialized (DIN: GPIO 33, BCK: GPIO 25, LCK: GPIO 32)\n");
}

// Generate a sine wave for a given frequency and amplitude
static void generate_sine_wave(int16_t *buffer, int samples_per_cycle, int amplitude)
{
    for (int i = 0; i < samples_per_cycle; i++)
    {
        buffer[i] = amplitude * sinf(2.0f * M_PI * i / samples_per_cycle);
    }
}

// Play a tone with a specific frequency and duration
void speaker_play_tone(int frequency, int duration_ms)
{
    if (!is_i2s_channel_enabled)
    {
        ESP_ERROR_CHECK(i2s_channel_enable(tx_channel));
        is_i2s_channel_enabled = true; // Update the flag
    }

    // Validate input parameters
    if (frequency <= 0 || duration_ms <= 0)
    {
        // printf("Invalid tone parameters: Frequency = %d, Duration = %d ms\n", frequency, duration_ms);
        return;
    }

    Settings *settings = settings_get(); // Get the current settings
    const int max_amplitude = 3000; // Maximum amplitude of the sine wave
    int amplitude = (max_amplitude * settings->volume) / 100; // Scale amplitude by volume (0-100%)
    const int samples_per_cycle = SAMPLE_RATE / frequency;
    int16_t sine_wave[samples_per_cycle];

    // printf("Generating sine wave: Frequency = %d Hz, Amplitude = %d\n", frequency, amplitude);
    generate_sine_wave(sine_wave, samples_per_cycle, amplitude);

    // Play the sine wave for the specified duration
    int total_samples = (SAMPLE_RATE * duration_ms) / 1000;
    int samples_played = 0;

    // printf("Playing tone: Frequency = %d Hz, Duration = %d ms, Volume = %d%%\n", frequency, duration_ms, settings->volume);

    while (samples_played < total_samples)
    {
        size_t bytes_written;
        esp_err_t err = i2s_channel_write(tx_channel, sine_wave, sizeof(sine_wave), &bytes_written, pdMS_TO_TICKS(100));
        if (err != ESP_OK)
        {
            // printf("I2S write error: %s\n", esp_err_to_name(err));
            break; 
        }

        samples_played += samples_per_cycle;

        // Print progress every 100 iterations
        if (samples_played % (samples_per_cycle * 100) == 0)
        {
            // printf("Progress: %d/%d samples played\n", samples_played, total_samples);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // printf("Finished playing tone: Frequency = %d Hz, Duration = %d ms\n", frequency, duration_ms);
}

// Play the currently selected signal
void speaker_play_signal(const Signal *signal)
{
    for (int i = 0; i < signal->tone_count; i++)
    {
        speaker_play_tone(signal->tones[i].frequency, signal->tones[i].duration);
    }
}

// Stop any ongoing audio playback
void speaker_stop(void)
{
    if (is_i2s_channel_enabled)
    {
        ESP_ERROR_CHECK(i2s_channel_disable(tx_channel));
        is_i2s_channel_enabled = false; // Update the flag
        // printf("Speaker playback stopped\n");
    }
    else
    {
        // printf("Speaker stop called, but I2S channel is not enabled.\n");
    }
}

// Update the speaker state based on the light state
void speaker_update(void)
{
    static int previous_light_state = -1; // Initialize to an invalid state
    //printf("Previous light state: %d\n", previous_light_state);
    static bool is_playing_signal = false; // Track if a signal is currently playing
    Settings *settings = settings_get(); // Get the current settings

    int current_light_state = settings->light; // Get the current light state

    // Check if the light state has changed
    if (current_light_state != previous_light_state)
    {
        // printf("Light state changed: %d -> %d\n", previous_light_state, current_light_state);

        // Stop the current signal if it's playing
        if (is_playing_signal)
        {
            speaker_stop();
            is_playing_signal = false;
        }

        // Play the new signal if sound is enabled
        if (settings->sound_on)
        {
            const Signal *current_signal = get_selected_signal();
            speaker_play_signal(current_signal); // Play the selected signal
            is_playing_signal = true; // Mark that a signal is now playing
        }

        previous_light_state = current_light_state; // Update the previous state
    }
}

void speaker_task(void *pvParameters)
{
    esp_task_wdt_add(NULL); // Register the task with the watchdog

    while (1)
    {
        Settings *settings = settings_get(); // Get the current settings

        // Check if the speaker is disabled
        if (!settings->sound_on)
        {
            printf("Speaker is disabled, entering sleep mode\n");
            esp_task_wdt_reset(); // Feed the watchdog
            vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 second
            continue;
        }

        // Call the speaker update function
        speaker_update();

        // Feed the watchdog
        esp_task_wdt_reset();

        // Delay to avoid busy looping
        vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms
    }

    esp_task_wdt_delete(NULL); // Unregister the task (if it ever exits)
}