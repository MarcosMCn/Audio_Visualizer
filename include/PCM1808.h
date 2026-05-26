#ifndef PCM1808_H
#define PCM1808_H

#include <stdint.h>
#include <stddef.h>
#include "driver/i2s_std.h"
#include "driver/gpio.h"

#define AUDIO_READ_SIZE 2048
#define SAMPLE_RATE 48000

extern i2s_chan_handle_t PCM1808_handle;
esp_err_t PCM1808_init(void);
size_t PCM1808_read(int32_t *buffer_left, int32_t *buffer_right);


#endif