/**
 * @file main.c
 * @author Jhonatas Anthony Dantas Araújo
 * @brief Simulação de um sensor de movimento usando o microfone da placa Raspberry Pi Pico W com o BitDogLab em C.
 * 
 * O microcontrolador monitora o ambiente e, ao detectar um ruído acima do normal, altera o status do sistema e aciona os LEDs e o buzzer conforme necessário.
 * 
 * @version 1.5.1
 * @date 2025-02-16
 * 
 * 
 * ## Histórico de Mudanças
 * - 1.0.0 - [11/02/2025] Primeira versão do código implementando o código do microfone
 * - 1.1.0 - [12/02/2025] Criação da função de envio de requisições para o servidor
 * - 1.2.0 - [13/02/2025] Implementa a lógica de detecção de movimento com base em ruídos externos
 * - 1.3.0 - [14/02/2025] Implementa a lógica de cores do LEDs conforme o status do sistema
 * - 1.4.0 - [15/02/2025] Implementa a lógica de envio de requisições para o servidor conforme o status do sistema
 * - 1.5.0 - [15/02/2025] Implementa o Buzzer quando o status do sistema está em Alarme
 * - 1.5.1 - [15/02/2025] Modulariza o código
 * - 1.6.0 - [16/02/2025] Comentários adicionados e limpeza de código
 */

#include <stdio.h>
#include "pico/time.h"
#include "pico/stdlib.h"

// Inclui os arquivos de cabeçalho criador pelo @author
#include "include/wifi.h"
#include "include/mic.h"
#include "include/buzzer.h"
#include "include/led.h"

#define WIFI_SSID       "Wedjhoze1" // Nome da rede
#define WIFI_PASS       "43900000"  // Senha da rede

#define LED_GREEN       11          // Pino do LED Verde
#define LED_RED         13          // Pino do LED Vermelho

#define NOTE_C5         523         // Dó (5ª oitava)
#define NOTE_D4         293         // Ré (4ª oitava)

#define BUZZER_PIN      21          // Pino do Buzzer
#define PWM_DIVIDER     16.0        // Divisor da frequência do PWM para controle do buzzer
#define PWM_PERIOD      4096        // Período do PWM usado para gerar sons no buzzer


#define ROOM_ID         1           // ID da sala (esse valor foi capturado pelo servidor após criar o cômodo) 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Estados do sistema
 * 
 * 1 = Silêncio
 * 2 = Barulho
 * 3 = Alarme
 * 
 */
int atualStatus = 1;

const uint notes[] = {NOTE_C5, NOTE_D4};    // Notas para o alarme
const uint note_duration[] = {500, 500};    // Duração das notas

bool resposta_enviada = false;              // Determina se a resposta foi enviada ou não


/**
 * @brief Programa principal
 * 
 * Simula um sensor de movimento com o microfone da placa Raspberry Pi Pico W e envia requisições para um servidor para alterar o status do sistema.
 * 
 * @details
 * O microcontrolador monitora o ambiente e, ao detectar um ruído acima do normal, altera o status do sistema e aciona os LEDs e o buzzer conforme necessário.
 * 
 */
int main()
{
    stdio_init_all();

    // A PRIXIMA LINHA DEVE SER DESCOMENTADA CASO SEJA NECESSÁRIO USAR O MONITOR SERIAL PARA DEPURAR O CÓDIGO
    // sleep_ms(5000);           // Delay para o usuário abrir o monitor serial

    uint32_t interval = 1000; // Intervalo de 1 segundo que vai ser utilizado para o temporizador

    /// Configuração do ADC
    adc_init_handler();

    /// Conecta ao Wi-Fi
    wifi_connect(WIFI_SSID, WIFI_PASS);

    /// Inicializa os LEDs
    init_leds();

    /// Setup do buzzer
    init_buzzer(BUZZER_PIN);

    // Inicializa os leds como verde
    set_led_status(LED_GREEN, 1);
    set_led_status(LED_RED, 0);

    // Define o temporizador de sistema como 10 segundos, as requisições serão enviadas a cada 10 segundos
    absolute_time_t next_wake_time = delayed_by_us(get_absolute_time(), interval * 10000);

    while (true)
    {
        sample_mic();
        float avg = mic_power();
        avg = 2.f * fabs(ADC_ADJUST(avg));

        if (avg > 0.1f && resposta_enviada == false)
        {
            // Determina o próximo status garantindo que não passe de 3
            printf("Movimento detectado: %8.4f V\n", avg);

            if (atualStatus == 1)
            {
                resposta_enviada = true;
                send_request_to_change_status(2);
                // Espera 10 segundos
                next_wake_time = delayed_by_us(get_absolute_time(), interval * 10000);
            }
        }

        if (time_reached(next_wake_time))
        {
            // Se em algum momento o admin colocar o status da casa como 1, ele vai para o status 1 após mandar a requisição
            if (atualStatus == 2 || atualStatus == 3)
            {
                if (resposta_enviada == false)
                {
                    resposta_enviada = true;
                    send_request_to_change_status(3);
                    next_wake_time = delayed_by_us(next_wake_time, interval * 10000);
                }
            }
        }
        else
        {
            while (!time_reached(next_wake_time) && atualStatus == 3)
            {
                for (int i = 0; i < (int)(sizeof(notes) / sizeof(notes[0])) && !time_reached(next_wake_time) && atualStatus == 3; i++)
                {
                    play_tone(BUZZER_PIN, notes[i], note_duration[i]);
                    if (atualStatus != 3)
                        break;
                }
                // No final de cada loop vai validar o status para ver se o admin mudou manualmente
                if (atualStatus != 3)
                    break;
            };
        }

        if (is_response_complete())
        {
            // Procura pelo início do corpo da resposta (após o header)
            char *body = strstr(get_response_buffer(), "\r\n\r\n");
            if (body != NULL)
            {
                body += 4;
                while (*body == ' ' || *body == '\n' || *body == '\r')
                    body++;

                // Agora, compara o valor retornado
                if (strcmp(body, "11") == 0)
                {
                    printf("Casa Aberta\n");
                    set_led_status(LED_GREEN, 1);
                    set_led_status(LED_RED, 0);
                    atualStatus = 1;
                }
                else if (strcmp(body, "12") == 0)
                {
                    printf("Mudança Humana \n");
                    set_led_status(LED_GREEN, 1);
                    set_led_status(LED_RED, 0);
                    atualStatus = 1;
                }
                else if (strcmp(body, "02") == 0 || strcmp(body, "03") == 0)
                {
                    if (strcmp(body, "02") == 0)
                    {
                        set_led_status(LED_GREEN, 1);
                        set_led_status(LED_RED, 1);
                        atualStatus = 2;
                    }
                    if (strcmp(body, "03") == 0)
                    {
                        set_led_status(LED_GREEN, 0);
                        set_led_status(LED_RED, 1);
                        atualStatus = 3;
                    }
                    printf("Requisição bem-sucedida (retorno %s)\n", body);
                }
                else
                {
                    printf("Código desconhecido retornado: %s\n", body);
                }
            }
            else
            {
                printf("Corpo da resposta não encontrado\n");
            }
            // Reseta o buffer e a flag para futuras requisições
            set_response_complete(false);
            resposta_enviada = false;
            set_response_buffer(NULL);
        }

        sleep_ms(100);
    }
}