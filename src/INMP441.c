#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "INMP441.h"
#include "esp_log.h"

static const char *TAG = "INMP441";


esp_err_t INMP441_init(void)
{
    // Configure I2S peripheral for INMP441 microphone module
    i2s_config_t INMP441_i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,   // receive mode
        .sample_rate = 48000,                    // audio sample rate
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4, // Set up 4 buffers in memory
        .dma_buf_len = 1024, // Each buffer holds 1024 samples.
        .use_apll = true,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    // Configure pins (adjust for your wiring)
    i2s_pin_config_t INMP441_pin_config = {
        .bck_io_num = 23,              // SCK
        .ws_io_num = 22,               // LRCLK
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 21              // SD (mic data)
    };

    ESP_LOGI(TAG, "Installing INMP441 I2S driver...");

    // Install driver
    i2s_driver_install(INMP441_I2S_PORT, &INMP441_i2s_config, 0, NULL);

    // Set pins
    i2s_set_pin(INMP441_I2S_PORT, &INMP441_pin_config);

    ESP_LOGI(TAG, "INMP441 initialized");

    return ESP_OK;
}

size_t INMP441_read(int32_t *buffer, size_t samples)
{
    size_t bytes_read = 0;

    esp_err_t ret = i2s_read(
        INMP441_I2S_PORT,
        buffer,
        samples * sizeof(int32_t),
        &bytes_read,
        portMAX_DELAY
    );

    if (ret != ESP_OK) {
        return 0;
    }

    return bytes_read / sizeof(int32_t);
}
