#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

void uart_init(void);
void test_task(void *param);

#endif