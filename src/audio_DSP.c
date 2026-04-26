#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "audio_streaming.h"
#include "INMP441.h"
#include "audio_streaming.h"
#include "audio_DSP.h"
#include "math.h"
#include "esp_dsp.h"

#define NORMALIZE (1.0f / 8388608.0f)
#define BANDS 64
#define SAMPLE_RATE 48000.0f

void audio_DSP_task(void *param)
{
    QueueHandle_t queue = (QueueHandle_t)param;
    fft_sample_buffer_t *raw_audio_data;

    while (1)
    {
        // 1. Receive data from queue
        if (xQueueReceive(queue, &raw_audio_data, portMAX_DELAY))
        {
            static float fft_samples[FFT_SAMPLE_BUFFER_SIZE*2];


            // Calculate RMS to measure "loudness" of sound.
            float rms = 0;
            for (int i = 0; i < raw_audio_data->sample_count; i++) {
            float sample = (float)(raw_audio_data->samples[i] >> 8) * NORMALIZE;
            rms += sample * sample;
            }
            rms = sqrtf(rms / raw_audio_data->sample_count);


            float db = 20.0f * log10f(rms + 1e-12f);
            static float db_smooth = 0.0f;
            float alpha = (db > db_smooth) ? 0.6f : 0.95f;
            db_smooth = alpha * db_smooth + (1 - alpha) * db;
            printf("Loudness: %.2f dB\n", db_smooth);

            // 2. Apply Hamming window and populate sample table.
            for (int i = 0; i < raw_audio_data->sample_count; i++) {
                float sample = (float)(raw_audio_data->samples[i] >> 8) * NORMALIZE;
                float hamming_window = 0.54f - 0.46f*cosf(2.0f*M_PI*i/(raw_audio_data->sample_count - 1));
                fft_samples[2*i] = sample * hamming_window;
                fft_samples[2*i + 1] = 0.0f;
            }
            
             // 3. FFT
            dsps_fft2r_fc32(fft_samples, FFT_SAMPLE_BUFFER_SIZE);

            // 4. Bit reversal
            dsps_bit_rev_fc32(fft_samples, FFT_SAMPLE_BUFFER_SIZE);

            // 5. Magnitude spectrum
            static float magnitude[FFT_SAMPLE_BUFFER_SIZE];
            for (int i = 0; i < FFT_SAMPLE_BUFFER_SIZE; i++) {
                float real = fft_samples[2 * i];
                float imag = fft_samples[2 * i + 1];
                magnitude[i] = sqrtf(real * real + imag * imag);
            }

            // // 6. Peak detection
            // float maxVal = 0;
            // int maxIndex = 0;

            // for (int i = 1; i < FFT_SAMPLE_BUFFER_SIZE; i++) {
            //     if (magnitude[i] > maxVal) {
            //         maxVal = magnitude[i];
            //         maxIndex = i;
            //     }
            // }
        }
    }
}
