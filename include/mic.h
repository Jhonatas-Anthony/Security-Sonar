#ifndef ADC_HANDLER_H
#define ADC_HANDLER_H

#include <math.h>
#include "hardware/adc.h"
#include "hardware/dma.h"

#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)
#define ADC_CLOCK_DIV 96.f
#define SAMPLES 200
#define ADC_ADJUST(x) (x * 3.3f / (1 << 12u) - 1.65f)
#define LIMIAR_DECIBEIS 0.5f

// uint dma_channel;
// dma_channel_config dma_cfg;

void adc_init_handler();
void sample_mic();
float mic_power();
// uint8_t get_intensity(float v);


#endif
