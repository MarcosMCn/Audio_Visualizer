#ifndef AUDIO_STREAMING_H
#define AUDIO_STREAMING_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "PCM1808.h"
#include "stream_buffer.h"

#define FFT_WINDOW_SIZE 2048

extern stream_buffer_t left_stream_buffer;
extern stream_buffer_t right_stream_buffer;


typedef struct {
    int32_t samples_left[AUDIO_READ_SIZE];
    int32_t samples_right[AUDIO_READ_SIZE];
    size_t sample_count;
} audio_stream_data_t;



extern TaskHandle_t audio_streaming_task_handle;
extern TaskHandle_t audio_DSP_task_handle;

void audio_streaming_task(void *param);
#endif