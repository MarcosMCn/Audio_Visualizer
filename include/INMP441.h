#ifndef INMP441_H
#define INMP441_H

#include <stdint.h>
#include <stddef.h>
#include "driver/i2s.h"
#include "esp_err.h"

#define INMP441_I2S_PORT I2S_NUM_0

esp_err_t INMP441_init(void);
size_t INMP441_read(int32_t *buffer, size_t samples);

#endif