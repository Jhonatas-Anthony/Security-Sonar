/**
 * @file mic.c
 * @brief Implementação das funções para captura e processamento de áudio com ADC e DMA no Raspberry Pi Pico.
 * 
 * Este módulo configura e utiliza o ADC e DMA para capturar dados do microfone, calcular a potência do sinal
 * e auxiliar na detecção de ruídos.
 * 
 * @author Jhonatas Anthony Dantas Araújo
 * @date 2025
 */

#include "include/mic.h"

/// Canal DMA utilizado para transferência dos dados do ADC.
uint dma_channel;

/// Configuração do canal DMA.
dma_channel_config dma_cfg;

/// Buffer onde os valores do ADC são armazenados.
uint16_t adc_buffer[SAMPLES];

/**
 * @brief Captura uma amostra do microfone utilizando ADC e DMA.
 *
 * A função realiza a leitura do ADC e armazena os valores no buffer através do DMA,
 * garantindo uma captura eficiente dos sinais de áudio.
 */
void sample_mic() {
    adc_fifo_drain();        // Limpa o FIFO do ADC
    adc_run(false);          // Garante que o ADC não esteja rodando
    dma_channel_configure(dma_channel, &dma_cfg,
        adc_buffer,
        &adc_hw->fifo,
        SAMPLES,
        true
    );

    // Inicia a captura do ADC
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_channel);  // Aguarda a finalização do DMA

    // Para a captura do ADC
    adc_run(false);
}

/**
 * @brief Calcula a potência do sinal capturado.
 * 
 * A função calcula a média quadrática das amostras do microfone,
 * fornecendo uma estimativa da intensidade do som.
 * 
 * @return Potência do sinal capturado.
 */
float mic_power() {
    float avg = 0.f;
    for (uint i = 0; i < SAMPLES; ++i)
        avg += adc_buffer[i] * adc_buffer[i];
    avg /= SAMPLES;
    return sqrt(avg);
}

/**
 * @brief Inicializa o ADC e configura o DMA para capturar sinais do microfone.
 *
 * Configura o ADC para operar no canal do microfone e define os parâmetros do DMA
 * para transferência eficiente dos dados.
 */
void adc_init_handler(){
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);
    adc_fifo_setup(
        true,       // Habilita o FIFO do ADC
        true,       // Habilita DMA
        1,          // Número de amostras antes de enviar ao FIFO
        false,      // Nenhuma ativação automática
        false       // Sem habilitação de erro
    );

    adc_set_clkdiv(ADC_CLOCK_DIV);

    // Configuração do DMA
    dma_channel = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
}