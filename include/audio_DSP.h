#ifndef AUDIO_DSP_H
#define AUDIO_DSP_H

#include <stdint.h>
#include <stddef.h>
#include "driver/i2s.h"
#include "esp_err.h"

void audio_DSP_task(void *param);

#endif