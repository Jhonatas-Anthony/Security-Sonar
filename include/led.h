#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include "pico/stdlib.h"

#define LED_RED 13
#define LED_GREEN 11

void init_leds();

void set_led_status(int pin, int state);

#endif
