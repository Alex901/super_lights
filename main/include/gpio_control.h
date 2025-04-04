#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

void gpio_init(void);
void gpio_set_led(int state);
int gpio_get_button_state(void);

#endif // GPIO_CONTROL_H