/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

int counter = 0;

void initilise_all()
{
    stdio_init_all();
    gpio_init(21);
    
    gpio_set_dir(21, GPIO_IN);
}

void gpio_callback(uint gpio, uint32_t events) {
    counter += 5;
    printf("Interrupt counter: %d\n", counter);
}

int main() {
    
    initilise_all();
    gpio_set_irq_enabled_with_callback(21, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    while (1) {
        counter += 3;
        printf("counter at 0x%x. Value: %d\n", &counter, counter);
        if(counter > 255)
            counter = 0;
        sleep_ms(2000);
    }

    return 0;
}
