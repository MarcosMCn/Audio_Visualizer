#ifndef AUDIO_STREAMING_H
#define AUDIO_STREAMING_H

#include <stdint.h>
#include <stddef.h>
#include "driver/i2s.h"
#include "esp_err.h"

#define AUDIO_READ_SIZE 1024
#define FFT_SAMPLE_BUFFER_SIZE 4096

typedef struct {
    int32_t samples[AUDIO_READ_SIZE];
    size_t sample_count;
} audio_stream_data_t;

typedef struct {
    int32_t samples[FFT_SAMPLE_BUFFER_SIZE];
    size_t sample_count;
} fft_sample_buffer_t;

void audio_streaming_task(void *param);

#endif