#include "include/led.h"
#include "pico/stdlib.h"

void init_leds() {
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
}

void set_led_status(int pin, int state) {
    gpio_put(pin, state);
}
