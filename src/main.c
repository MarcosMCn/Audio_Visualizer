#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "PCM1808.h"
#include "audio_streaming.h"
#include "audio_DSP.h"
#include "esp_dsp.h"
#include "driver/uart.h"
#include "main.h"
#include "stream_buffer.h"

static const char *TAG = "MAIN";

void test_task(void *param)
{
    while (1) {
        printf("alive\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_NONE);
    uart_init();
    dsps_fft2r_init_fc32(NULL, FFT_WINDOW_SIZE);

    PCM1808_init();

    stream_buffer_init(&left_stream_buffer);
    stream_buffer_init(&right_stream_buffer);

    // Define audio streaming task
    xTaskCreatePinnedToCore(
        audio_streaming_task, // Task function
        "audio_stream", // Task name
        8196, // Stack size
        NULL, // Function parameter
        7, // Task priority
        &audio_streaming_task_handle, // Task handle
        1 // CPU core
    );

    // Define audio processing task
    xTaskCreatePinnedToCore(
        audio_DSP_task,
        "audio_dsp",
        8196,
        NULL,
        7,
        &audio_DSP_task_handle,
        1
    );

}

void uart_init(void)
{
    uart_config_t config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM_0, &config);
    uart_driver_install(UART_NUM_0, 2048, 0, 0, NULL, 0);
}


