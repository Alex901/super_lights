#ifndef SPEAKER_CONTROL_H
#define SPEAKER_CONTROL_H

#include "settings_control.h"

// Initialize the speaker
void speaker_init(void);

// Play a tone with a specific frequency and duration
void speaker_play_tone(int frequency, int duration_ms);

// Play the currently selected signal
void speaker_play_signal(const Signal *signal);

// Stop any ongoing audio playback
void speaker_stop(void);

// Update the speaker state based on settings
void speaker_update(void); 

// Task function for speaker control
void speaker_task(void *pvParameters); 

#endif // SPEAKER_CONTROL_H