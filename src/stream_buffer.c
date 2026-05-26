#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "audio_streaming.h"
#include "PCM1808.h"
#include <string.h>
#include "stream_buffer.h"

stream_buffer_t left_stream_buffer;
stream_buffer_t right_stream_buffer;

void stream_buffer_init(stream_buffer_t *sb)
{
    // Set all values to 0
    memset(sb, 0, sizeof(*sb));
    sb->mutex = xSemaphoreCreateMutex();
    if (sb->mutex == NULL)
    {
        printf("Failed to create stream buffermutex\n");
        return;
    }
}

void stream_buffer_write(stream_buffer_t *sb, int32_t *read_array, size_t len)
{
    size_t len_to_end = STREAM_BUFFER_SIZE - sb->write_pos;

    if (len <= len_to_end) {
        memcpy(sb->buffer + sb->write_pos, read_array, len * sizeof(int32_t)); // Copy memory into buffer from array, starting at the write position.
        sb->write_pos += len;

        if(sb->write_pos == STREAM_BUFFER_SIZE) {
            sb->write_pos = 0;
        }
    }
    else {
        // First chunk: to end of buffer
        memcpy(sb->buffer + sb->write_pos, read_array, len_to_end * sizeof(int32_t));

        // Second chunk: wrap to beginning
        memcpy(sb->buffer, read_array + len_to_end, (len - len_to_end)*sizeof(int32_t));

        sb->write_pos = len - len_to_end;
    }

}

void stream_buffer_read(stream_buffer_t *sb, int32_t *write_array, size_t len) {

    uint32_t read_pos = (sb->write_pos - len + STREAM_BUFFER_SIZE) % STREAM_BUFFER_SIZE;
    size_t len_to_end = STREAM_BUFFER_SIZE - read_pos;

    if (len <= len_to_end) {
        memcpy(write_array, sb->buffer + read_pos, len * sizeof(int32_t)); // Copy memory into buffer from array, starting at the write position.
    }
    else {
        memcpy(write_array, sb->buffer + read_pos, len_to_end * sizeof(int32_t));
        memcpy(write_array + len_to_end, sb->buffer, (len - len_to_end) * sizeof(int32_t));
    }

}