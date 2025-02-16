
#include "include/buzzer.h"

#define BUZZER_PIN 21
#define NOTE_C5 523
#define NOTE_D4 293
#define PWM_DIVIDER 16.0

static const uint notes[] = {NOTE_C5, NOTE_D4};
static const uint note_duration[] = {500, 500};

void init_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, PWM_DIVIDER);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

void play_alarm() {
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    for (int i = 0; i < 2; i++) {
        uint32_t top = clock_get_hz(clk_sys) / (notes[i] * 16);
        pwm_set_wrap(slice, top);
        pwm_set_chan_level(slice, PWM_CHAN_A, top / 2);
        pwm_set_enabled(slice, true);
        sleep_ms(note_duration[i]);
    }
    pwm_set_enabled(slice, false);
}

void play_tone(uint pin, uint frequency, uint duration_ms)
{
    if (frequency == 0)
    {
        sleep_ms(duration_ms);
        return;
    }

    uint slice = pwm_gpio_to_slice_num(pin);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = clock_freq / (frequency * 16); // O divisor de clock já está definido como 16

    pwm_set_wrap(slice, top);
    pwm_set_chan_level(slice, PWM_CHAN_A, top / 2); // 50% de duty cycle
    pwm_set_gpio_level(pin, top / 2);
    pwm_set_enabled(slice, true);

    sleep_ms(duration_ms);

    pwm_set_enabled(slice, false); // Para o som após a duração
}
