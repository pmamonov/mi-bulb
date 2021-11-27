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

#include "main.h"

static const char *TAG = "MAIN";

volatile unsigned long br = 90;

// PWM period 1000us(1Khz), same as depth
#define PWM_PERIOD    (1000)
#define PWM_PIN     5

void app_main()
{
    uint32_t pin = PWM_PIN;
    uint32_t duty = PWM_PERIOD * br / 100;
    float phase = 0;
    int br_old = br;

    pwm_init(PWM_PERIOD, &duty, 1, &pin);
    pwm_set_phases(&phase);
    pwm_start();

    start_http();

    while (1) {
        if (br != br_old) {
            duty = PWM_PERIOD * br / 100;
            br_old = br;
            ESP_LOGI(TAG, "%d\n", duty);
            pwm_set_duties(&duty);
            pwm_start();
        }

        vTaskDelay(configTICK_RATE_HZ / 10);
    }
}

