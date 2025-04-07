#include "button_control.h"
#include "gpio_control.h"
#include "esp_rom_sys.h"

int button_is_pressed(int gpio_num)
{
    if (gpio_get_button_state(gpio_num) == 0)
    {
        // Simple software filter: Confirm button press is stable
        esp_rom_delay_us(5000); // Delay for 5 milliseconds
        if (gpio_get_button_state(gpio_num) == 0)
        {
            // Wait for button to be released (debouncing)
            while (gpio_get_button_state(gpio_num) == 0)
            {
                esp_rom_delay_us(1000); // Delay for 1 millisecond
            }
            return 1; // Button press confirmed
        }
    }
    return 0; // Button not pressed
}