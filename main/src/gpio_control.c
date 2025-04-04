#include "gpio_control.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

// GPIO definitions
#define POWER_LED_GPIO GPIO_NUM_2
#define POWER_BUTTON_GPIO GPIO_NUM_0

void gpio_init(void)
{
    // Configure LED GPIO as output
    gpio_config_t io_conf_led = {
        .pin_bit_mask = (1ULL << POWER_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf_led);

    // Configure Button GPIO as input with pull-up resistor
    gpio_config_t io_conf_button = {
        .pin_bit_mask = (1ULL << POWER_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf_button);
}

void gpio_set_led(int state)
{
    gpio_set_level(POWER_LED_GPIO, state);
}

int gpio_get_button_state(void)
{
    return gpio_get_level(POWER_BUTTON_GPIO);
}