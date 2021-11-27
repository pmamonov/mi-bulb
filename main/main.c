/* pwm example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

#include "driver/pwm.h"

static const char *TAG = "pwm_example";

// PWM period 1000us(1Khz), same as depth
#define PWM_PERIOD    (1000)
#define PWM_PIN     5

void app_main()
{
    uint32_t pin = PWM_PIN;
    uint32_t duty = 0;
    float phase = 0;

    pwm_init(PWM_PERIOD, &duty, 1, &pin);
    pwm_set_phases(&phase);
    pwm_start();

    while (1) {
        ESP_LOGI(TAG, "%d\n", duty);
        pwm_set_duties(&duty);
        pwm_start();
        vTaskDelay(configTICK_RATE_HZ);
#define STEP    (PWM_PERIOD / 10)
        duty = (duty + STEP) % (PWM_PERIOD + STEP);
    }
}

