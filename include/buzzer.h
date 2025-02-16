#ifndef BUZZER_HANDLER_H
#define BUZZER_HANDLER_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

void init_buzzer();
void play_alarm();
void play_tone();

#endif