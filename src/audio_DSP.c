#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "audio_DSP.h"
#include "math.h"
#include "esp_dsp.h"
#include "driver/uart.h"
#include "audio_streaming.h"
#include "audio_DSP.h"

#define NORMALIZE (1.0f / 8388608.0f)
#define FFT_HALF_SIZE (FFT_WINDOW_SIZE / 2)

float alpha = 0.9974f;
float x_prev = 0.0f;
float y_prev = 0.0f;

TaskHandle_t audio_DSP_task_handle = NULL;

void audio_DSP_task(void *param)
{
    float max_magnitude = 0;
    static float magnitude_EMA[FFT_HALF_SIZE];
    float EMA_smoothing_factor = 0.5;
    static int32_t fft_left_window[FFT_WINDOW_SIZE];

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        xSemaphoreTake(left_stream_buffer.mutex, portMAX_DELAY);
        stream_buffer_read(&left_stream_buffer, fft_left_window, FFT_WINDOW_SIZE);
        xSemaphoreGive(left_stream_buffer.mutex);
        static float fft_samples[FFT_WINDOW_SIZE*2]; // FFT samples must store data in Re, Im, Re, Im complex interleaved format.
        
        float sum = 0;
        for (int i = 0; i < FFT_WINDOW_SIZE; i++) {
            float sample = (float)(fft_left_window[i]);            
            sum += sample;
        }

        float mean = sum / FFT_WINDOW_SIZE;

        for (int i = 0; i < FFT_WINDOW_SIZE; i++) {
            float sample = (float)(fft_left_window[i]) - mean;
            sample = highPass_filter(sample);
            float hamming_window = 0.54f - 0.46f*cosf(2.0f*M_PI*i/(FFT_WINDOW_SIZE - 1));
            fft_samples[2*i] = sample * hamming_window;
            fft_samples[2*i + 1] = 0.0f;
        }

        dsps_fft2r_fc32(fft_samples, FFT_WINDOW_SIZE);

        dsps_bit_rev_fc32(fft_samples, FFT_WINDOW_SIZE);

        // 5. Magnitude spectrum. We only compute the magnitudes for one half of our FFT samples because the rest is mirror negative.
        static float magnitude[FFT_WINDOW_SIZE/2];
        static int8_t mag_norm[FFT_WINDOW_SIZE/2];

        for (int i = 0; i < FFT_WINDOW_SIZE/2; i++) {
            float real = fft_samples[2 * i];
            float imag = fft_samples[2 * i + 1];
            magnitude[i] = sqrtf(real * real + imag * imag);
            if (i == 0) {
                magnitude[i] = 0;
            }
                
            magnitude_EMA[i] = EMA_smoothing_factor*magnitude[i] + (1.0f-EMA_smoothing_factor)*magnitude_EMA[i];
            if (magnitude_EMA[i] > max_magnitude) {
                max_magnitude = magnitude_EMA[i];
                }
        }

        if (max_magnitude < 1e-12f) {
            max_magnitude = 1e-12f;
        }

        for (int i = 0; i < FFT_WINDOW_SIZE/2; i++) {
            mag_norm[i] = (int8_t)(20.0f * log10f(magnitude_EMA[i] / max_magnitude) + 1e-6f);
        }

        // 6. Send all bin data over UART
        UART_transmit_FFT_data(mag_norm);
        
    }
}

void UART_transmit_FFT_data(int8_t *mag)
{
    static int8_t FFT_data_packet[2 + FFT_HALF_SIZE];

    FFT_data_packet[0] = 0xAB;
    FFT_data_packet[1] = 0xCD;

    memcpy(&FFT_data_packet[2], mag, FFT_HALF_SIZE);

    uart_write_bytes(UART_NUM_0, (const char*)FFT_data_packet, sizeof(FFT_data_packet));
}

void init_bin_freq(float* bins)
{
    for (int j = 0; j < FFT_WINDOW_SIZE; j++)
    {
        bins[j] = (SAMPLE_RATE / FFT_WINDOW_SIZE) * j;
    }
}

float highPass_filter(float x) {
    float y = alpha * (y_prev + x - x_prev);

    x_prev = x;
    y_prev = y;

    return y;
}


