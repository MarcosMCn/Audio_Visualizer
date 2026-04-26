#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "audio_streaming.h"
#include "INMP441.h"
#include "audio_streaming.h"


void audio_streaming_task(void *param)
{
    QueueHandle_t queue = (QueueHandle_t)param;

    static fft_sample_buffer_t fft_sample_buffer;  // sample buffer
    fft_sample_buffer_t *ptr = &fft_sample_buffer; // Pointer to our sample buffer
    static int write_index = 0;

    audio_stream_data_t audio_data;

    while (1)
    {
        // Read 1024 samples from I2S
        audio_data.sample_count = INMP441_read(audio_data.samples, AUDIO_READ_SIZE);

        if (audio_data.sample_count == AUDIO_READ_SIZE) {
            
            // Copy into sample buffer
            for (int i = 0; i < AUDIO_READ_SIZE; i++) {

                fft_sample_buffer.samples[write_index] = audio_data.samples[i];
                write_index++;

                // When sample buffer is full, send address of sample buffer to message box/queue
                if (write_index >= FFT_SAMPLE_BUFFER_SIZE) {
                    fft_sample_buffer.sample_count = FFT_SAMPLE_BUFFER_SIZE;
                    xQueueSend(queue, &ptr, portMAX_DELAY);
                    write_index = 0;  // reset
                }

            }
        }
    }
}
