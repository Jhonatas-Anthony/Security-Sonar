#include "include/mic.h"

uint dma_channel;
dma_channel_config dma_cfg;
uint16_t adc_buffer[SAMPLES];

void sample_mic();
float mic_power();
uint8_t get_intensity(float v);

void sample_mic() {
    adc_fifo_drain();
    adc_run(false);
    dma_channel_configure(dma_channel, &dma_cfg,
        adc_buffer,
        &adc_hw->fifo,
        SAMPLES,
        true
    );
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    adc_run(false);
}

float mic_power() {
    float avg = 0.f;
    for (uint i = 0; i < SAMPLES; ++i)
        avg += adc_buffer[i] * adc_buffer[i];
    avg /= SAMPLES;
    return sqrt(avg);
}

void adc_init_handler(){
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);
    adc_fifo_setup(
        true,
        true,
        1,
        false,
        false
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