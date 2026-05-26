#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

extern stream_buffer_t left_stream_buffer;
extern stream_buffer_t right_stream_buffer;

void uart_init(void);
void test_task(void *param);

#endif