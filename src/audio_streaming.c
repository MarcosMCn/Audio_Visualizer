#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "audio_streaming.h"
#include "PCM1808.h"


TaskHandle_t audio_streaming_task_handle = NULL;

void audio_streaming_task(void *param)
{
    static audio_stream_data_t audio_data;

    while (1)
    {

        // Read samples
        audio_data.sample_count = PCM1808_read(audio_data.samples_left, audio_data.samples_right);

        if (audio_data.sample_count == AUDIO_READ_SIZE) {
            xSemaphoreTake(left_stream_buffer.mutex, portMAX_DELAY);
            stream_buffer_write(&left_stream_buffer, audio_data.samples_left, audio_data.sample_count);          
            xSemaphoreGive(left_stream_buffer.mutex);
            xTaskNotifyGive(audio_DSP_task_handle);
        }
    }
}

