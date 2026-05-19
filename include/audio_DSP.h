#ifndef AUDIO_DSP_H
#define AUDIO_DSP_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

void audio_DSP_task(void *param);
float highPass_filter(float x);
void init_bin_freq();
void UART_transmit_FFT_data(uint8_t *mag);
#endif