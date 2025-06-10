#ifndef GPIO_H
#define GPIO_H

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "esp_chip_info.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_timer.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/mcpwm_types.h"

// *** Pinout ***
#define MAIN_VALVE_PIN       46 // 46
#define PURGE_VALVE_PIN      45  // 45
#define MOSFET_PIN           21 // 21 driver enable // todo
#define LED_PIN              12 // 12 led CAN Status
#define EMERGENCY_BUTTON_PIN 1 // Button on/off signal //todo

void gpio_init(void);
void gpio_on_loop(void);

#endif // GPIO_H