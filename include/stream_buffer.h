#ifndef STREAMING_BUFFER_H
#define STREAMING_BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#define STREAM_BUFFER_SIZE 8192

typedef struct {
    int32_t buffer[STREAM_BUFFER_SIZE];
    uint32_t write_pos;
    SemaphoreHandle_t mutex;
} stream_buffer_t;

void stream_buffer_init(stream_buffer_t *sb);
void stream_buffer_write(stream_buffer_t *sb, int32_t *read_array, size_t len);
void stream_buffer_read(stream_buffer_t *sb, int32_t *write_array, size_t len);
#endif