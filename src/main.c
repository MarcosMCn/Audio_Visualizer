#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "INMP441.h"
#include "audio_streaming.h"
#include "audio_DSP.h"
#include "esp_dsp.h"

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
    dsps_fft2r_init_fc32(NULL, AUDIO_READ_SIZE);

    QueueHandle_t audio_queue;
    INMP441_init();

    audio_queue = xQueueCreate(2, sizeof(fft_sample_buffer_t *));
    if (audio_queue == NULL) {
        printf("Failed to create audio queue\n");
        return;
    }

    // Define audio streaming task
    xTaskCreatePinnedToCore(
        audio_streaming_task, // Task function
        "audio_stream", // Task name
        8196, // Stack size
        (void*)audio_queue, // Function parameter
        7, // Task priority
        NULL, // Task handle
        1 // CPU core
    );

    // Define audio processing task
    xTaskCreatePinnedToCore(
        audio_DSP_task,
        "audio_dsp",
        20480,
        (void *)audio_queue,
        5,
        NULL,
        1
    );

}


