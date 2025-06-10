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

#define portTICK_PERIOD_MS ((TickType_t) 1000 / configTICK_RATE_HZ)
#define configTICK_RATE_HZ (CONFIG_FREERTOS_HZ)
#define CONFIG_FREERTOS_HZ 100

#include "adc.h"
#include "gpio.h"
#include "pwm.h"
#include "console.h"
#include "timer.h"
#include "can.h"
#include "uart.h"

#include "fuel_cell_control.h"

#include "driver/uart.h"

void app_main()

{
    gpio_init();
    adc_init();
    pwm_init();
    //can_init(); //error braki
    // uart_init();
    console_init();
    //  temp
    //  pwm_set_pwm_duty_cycle(0);
    while (1)
    {
        adc_on_loop();
        fc_on_loop();
        // can_send();
        //   const char* testText = "brumbrum";
        //   uart_send_data(testText, 8);
        //   uint8_t buffer[8];
        //   size_t length = 8;
        //   TickType_t timeout = 100;
        //   uart_receive_data(buffer, length, timeout);
        //  can_recieve();
        //vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
