/**
 * @file led.c
 * @brief Implementação das funções para controle dos LEDs.
 * 
 * Este arquivo contém as funções responsáveis por inicializar e controlar
 * os LEDs do sistema.
 * 
 * @author Jhonatas Anthony Dantas Araújo
 * @date 2025
 */

#include "include/led.h"

/**
 * @brief Inicializa os pinos dos LEDs.
 * 
 * Configura os pinos dos LEDs como saída para que possam ser controlados.
 */
void init_leds() {
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
}

/**
 * @brief Define o estado de um LED.
 * 
 * @param pin Pino correspondente ao LED.
 * @param state Estado do LED (1 para ligado, 0 para desligado).
 */
void set_led_status(int pin, int state) {
    gpio_put(pin, state);
}
