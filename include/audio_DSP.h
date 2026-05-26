#ifndef AUDIO_DSP_H
#define AUDIO_DSP_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "stream_buffer.h"

extern TaskHandle_t audio_DSP_task_handle;

extern stream_buffer_t left_stream_buffer;
extern stream_buffer_t right_stream_buffer;

void audio_DSP_task(void *param);
float highPass_filter(float x);
void init_bin_freq();
void UART_transmit_FFT_data(int8_t *mag);
#endif