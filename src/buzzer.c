/**
 * @file buzzer.c
 * @brief Implementação das funções para controle do buzzer.
 * 
 * Este arquivo contém funções para inicializar e tocar sons no buzzer
 * utilizando PWM.
 * 
 * @author Jhonatas Anthony Dantas Araújo
 * @date 2025
 */

#include "include/buzzer.h"

/// Pino ao qual o buzzer está conectado
#define BUZZER_PIN      21

/// Frequência da nota Dó (C5) em Hz
#define NOTE_C5         523

/// Frequência da nota Ré (D4) em Hz.
#define NOTE_D4         293

/// Divisor de clock do PWM para controle do buzzer.
#define PWM_DIVIDER     16.0

/**
 * @brief Inicializa o buzzer configurando o PWM.
 * 
 * Define o pino do buzzer como saída PWM e configura o divisor de clock.
 */
void init_buzzer() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, PWM_DIVIDER);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

/**
 * @brief Toca um tom específico no buzzer por um determinado tempo.
 * 
 * @param pin Pino ao qual o buzzer está conectado.
 * @param frequency Frequência do tom em Hz.
 * @param duration_ms Duração do tom em milissegundos.
 */
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
