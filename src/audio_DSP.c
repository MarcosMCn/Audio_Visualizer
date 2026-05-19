#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "audio_streaming.h"
#include "audio_streaming.h"
#include "audio_DSP.h"
#include "math.h"
#include "esp_dsp.h"
#include "driver/uart.h"

#define NORMALIZE (1.0f / 8388608.0f)
#define FFT_HALF_SIZE (FFT_SAMPLE_BUFFER_SIZE / 2)

float alpha = 0.9974f;
float x_prev = 0.0f;
float y_prev = 0.0f;


void audio_DSP_task(void *param)
{
    QueueHandle_t queue = (QueueHandle_t)param;
    fft_sample_buffer_t *raw_audio_data;

    // Initialize frequency bins
    static float freq_bins[FFT_SAMPLE_BUFFER_SIZE];
    init_bin_freq(freq_bins);

    float max_magnitude = 0;
    static float magnitude_EMA[FFT_SAMPLE_BUFFER_SIZE/2];
    float EMA_smoothing_factor = 0.2;
    while (1)
    {
        // 1. Receive data from queue
        if (xQueueReceive(queue, &raw_audio_data, portMAX_DELAY))
        {
            static float fft_samples[FFT_SAMPLE_BUFFER_SIZE*2]; // FFT samples must store data in Re, Im, Re, Im complex interleaved format.

            // // Calculate RMS to measure "loudness" of sound.
            // float rms = 0;
            // float sum = 0;
            // for (int i = 0; i < raw_audio_data->sample_count; i++) {
            //     float sample = (float)(raw_audio_data->samples[i]);
            //     sample = highPass_filter(sample);               
            //     sum += sample;
            // }
            // float mean = sum / raw_audio_data->sample_count;
            //printf("Mean = %f\n", mean);
            //     float sample = (float)(raw_audio_data->samples[i]);
            //     rms += sample * sample;
            // }
            
            // rms = sqrtf(rms / raw_audio_data->sample_count);
            // float dbFS = 20*log10f(rms / 8388607.0f);


            // 2. Apply High pass Filter and Hamming window and populate sample table.
            for (int i = 0; i < raw_audio_data->sample_count; i++) {
                float sample = (float)(raw_audio_data->samples[i]);
                sample = highPass_filter(sample);
                float hamming_window = 0.54f - 0.46f*cosf(2.0f*M_PI*i/(raw_audio_data->sample_count - 1));
                fft_samples[2*i] = sample * hamming_window;
                fft_samples[2*i + 1] = 0.0f;
            }
            
             // 3. FFT
            dsps_fft2r_fc32(fft_samples, FFT_SAMPLE_BUFFER_SIZE);

            // 4. Bit reversal
            dsps_bit_rev_fc32(fft_samples, FFT_SAMPLE_BUFFER_SIZE);

            // 5. Magnitude spectrum. We only compute the magnitudes for one half of our FFT samples because the rest is mirror negative.
            static float magnitude[FFT_SAMPLE_BUFFER_SIZE/2];
            static uint8_t mag_norm[FFT_SAMPLE_BUFFER_SIZE/2];
            for (int i = 0; i < FFT_SAMPLE_BUFFER_SIZE/2; i++) {
                float real = fft_samples[2 * i];
                float imag = fft_samples[2 * i + 1];
                magnitude[i] = sqrtf(real * real + imag * imag);
                if (i == 0) {
                    magnitude[i] = 0;
                }
                
                magnitude_EMA[i] = EMA_smoothing_factor*magnitude[i] + (1.0f-EMA_smoothing_factor)*magnitude_EMA[i];
                if (magnitude_EMA[i] > max_magnitude) {
                    max_magnitude = magnitude[i];
                    }
            }

            if (max_magnitude < 1e-12f) {
                max_magnitude = 1e-12f;
            }

            for (int i = 0; i < FFT_SAMPLE_BUFFER_SIZE/2; i++) {
                mag_norm[i] = (uint8_t)((magnitude_EMA[i] / max_magnitude) * 255.0f);
            }

            // 6. Send all bin data over UART
            UART_transmit_FFT_data(mag_norm);

        }
    }
}

void UART_transmit_FFT_data(uint8_t *mag)
{
    static uint8_t FFT_data_packet[2 + FFT_HALF_SIZE];

    FFT_data_packet[0] = 0xAB;
    FFT_data_packet[1] = 0xCD;

    memcpy(&FFT_data_packet[2], mag, FFT_HALF_SIZE);

    uart_write_bytes(UART_NUM_0, (const char*)FFT_data_packet, sizeof(FFT_data_packet));
}

void init_bin_freq(float* bins)
{
    for (int j = 0; j < FFT_SAMPLE_BUFFER_SIZE; j++)
    {
        bins[j] = (SAMPLE_RATE / FFT_SAMPLE_BUFFER_SIZE) * j;
    }
}

float highPass_filter(float x) {
    float y = alpha * (y_prev + x - x_prev);

    x_prev = x;
    y_prev = y;

    return y;
}


