#ifndef ADC_H
#define ADC_H

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

// ADC channels
#define ADC_V_USB_CHANNEL        ADC_CHANNEL_3 // GPIO 13,ADC 2, CH 3 V USB // test & add coefficients
#define ADC_V_FC_CHANNEL         ADC_CHANNEL_7 // GPIO 4, ADC 1 CH 7
#define ADC_T_CHANNEL            ADC_CHANNEL_9 // GPIO 2, ADC 1 CH 1
#define ADC_P_CHANNEL            ADC_CHANNEL_4 // GPIO 3, ADC 1 CH 6 LP1
#define ADC_BUTTON_STATE_CHANNEL ADC_CHANNEL_0 // GPIO 13 ADC 2 CH 0 //G11 test MCU button on pcb // todo
#define ADC_CURRENT_FLOW_CHANNEL ADC_CHANNEL_6 // GPIO 7, ADC1 CH 4
#define ADC_V_SC_CHANNEL         ADC_CHANNEL_8 // GPIO 4, ADC 1 CH 8 // test
// #define ADC_MCU_TEST_CHANNEL   ADC_CHANNEL_1 // MCU test GPIO 1, ADC 1 CH 0

#define ADC_V_USB_SAMPLES_COUNT        10
#define ADC_V_FC_SAMPLES_COUNT         32
#define ADC_T_SAMPLES_COUNT            32
#define ADC_P_SAMPLES_COUNT            32
#define ADC_BUTTON_SAMPLES_COUNT       1 // test new capacitor and change to simple IO
#define ADC_CURRENT_FLOW_SAMPLES_COUNT 32
#define ADC_V_SC_SAMPLES_COUNT         32

#define ADC_60V_VOLTAGE_COEFF_COUNT  4
#define ADC_3V3_VOLTAGE_COEFF_COUNT  6
#define ADC_TEMPERATURE_COEFF_COUNT  2
#define ADC_CURRENT_FLOW_COEFF_COUNT 3

extern float V_FC_filtered_raw;
extern float V_FC_value;

extern float T_filtered_raw;
extern float T_value;

extern float P_filtered_raw;
extern float P_value;

extern int current_flow_raw;
extern float current_flow_filtered_raw;
extern float current_flow_value;

extern float button_state_filtered_raw;
extern float previous_button_state_filtered_raw;
extern float button_state_value;
extern float previous_button_state_value;

extern bool button_state;
extern bool previous_button_state;

extern float V_SC_filtered_raw;
extern float V_SC_value;

extern float V_USB_filtered_raw;
extern float V_USB_value;

float adc_map(float x, float in_min, float in_max, float out_min, float out_max);
float adc_apply_calibration(float coefficients[], uint8_t coeff_count, float adc_raw_sample); 
//float adc_apply_calibration(float coefficients[], uint8_t coeff_count, int adc_raw);//old

void adc_on_loop();
void adc_init();

#endif // ADC_H