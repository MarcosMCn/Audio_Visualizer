#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "PCM1808.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"

static const char *TAG = "PCM1808";
i2s_chan_handle_t PCM1808_handle = NULL;

esp_err_t PCM1808_init(void)
{

    i2s_chan_config_t PCM1808_chan_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    PCM1808_chan_config.auto_clear = true; // Auto clear the legacy data in the DMA buffer
    esp_err_t chan = i2s_new_channel(&PCM1808_chan_config, NULL, &PCM1808_handle);
    if (chan != ESP_OK) {
        return 0;
    }

    // Configure I2S peripheral for PCM1808 ADC module
    i2s_std_config_t PCM1808_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_3, // Must be 0, 1, or 3.
            .bclk = GPIO_NUM_23,
            .ws = GPIO_NUM_22,
            .dout = I2S_GPIO_UNUSED,
            .din = GPIO_NUM_21,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    PCM1808_config.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;

    // Initialize the I2S channel
    esp_err_t init = i2s_channel_init_std_mode(PCM1808_handle, &PCM1808_config);
    if (init != ESP_OK) {
        return 0;
    }

    // Set pins
    esp_err_t enable = i2s_channel_enable(PCM1808_handle);
    if (enable != ESP_OK) {
        return 0;
    }

    return ESP_OK;
}

size_t PCM1808_read(int32_t *buffer_left, int32_t *buffer_right)
{
    static int32_t read_buffer[AUDIO_READ_SIZE*2];
    size_t bytes_read = 0;

    esp_err_t ret = i2s_channel_read(
        PCM1808_handle,
        read_buffer,
        AUDIO_READ_SIZE * 2 * sizeof(int32_t),
        &bytes_read,
        portMAX_DELAY
    );

    if (ret != ESP_OK) {
        return 0;
    }

    size_t frames_read = bytes_read / (2 * sizeof(int32_t));

    for (size_t i = 0; i < frames_read; i++) {
        int32_t left_raw  = read_buffer[2 * i];
        int32_t right_raw = read_buffer[2 * i + 1];

        // Convert from PCM1808 24-bit (usually left-aligned in 32-bit)
        buffer_left[i]  = left_raw >> 8;
        buffer_right[i] = right_raw >> 8;
    }

    return frames_read;
}
