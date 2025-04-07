#include "gpio_control.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

void gpio_init(void)
{
    // Configure LED GPIO as output
    gpio_config_t io_conf_led = {
        .pin_bit_mask = (1ULL << POWER_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf_led);

    // Configure Button GPIOs as input with pull-up resistors
    gpio_config_t io_conf_button = {
        .pin_bit_mask = (1ULL << POWER_BUTTON_GPIO) | (1ULL << ENTER_BUTTON_GPIO) |
                        (1ULL << BACK_BUTTON_GPIO) | (1ULL << UP_BUTTON_GPIO) |
                        (1ULL << DOWN_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf_button);
}

void gpio_set_led(int state)
{
    gpio_set_level(POWER_LED_GPIO, state);
}

int gpio_get_button_state(int gpio_num)
{
    return gpio_get_level(gpio_num);
}