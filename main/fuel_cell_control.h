#ifndef FUEL_CELL_CONTROL_H
#define FUEL_CELL_CONTROL_H

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

#define PURGE_DURATION_MS             300 //0,3s
#define PURGE_MOSFET_DELAY_MS         5000  // 5s
#define MOSFET_SHORT_DURATION_MS      25    // 15ms
#define MOSFET_SHORT_INTERVAL_MS      50    // 40ms
#define FC_V_PROBING_TIME             60    // 60s, 1 minute na spadek napiecia o trigger difference
#define FC_V_PROBING_FREQUENCY        1  // 1Hz, 1s per sample
#define FC_V_PURGE_TRIGGER_DIFFERENCE 5.0 //spadek napiecia ogniwa inicjujący przedmuch
#define FC_MINIMAL_VOLTAGE            25.0 //todo mesurments
#define SC_MINIMAL_VOLTAGE            17.0

extern bool is_emergency;
extern bool fan_toggle_trigger;

extern float FC_V_buffer[FC_V_PROBING_TIME * FC_V_PROBING_FREQUENCY];
extern uint64_t FC_V_probing_timer;
extern int FC_V_buffer_current_idx;

// Start value
extern bool main_valve_toggle;  // ================= ustawienie zaworu zasilajacego _ main valve n-mosfet (1 on, 0 off)
extern bool purge_valve_toggle; // ================= ustawienie przedmuchu - purge valve n-mosfet (1 on, 0 off)
extern bool mosfet_toggle;      // ================= ustawienie zwarcia mosfet - driver i n-mosfet (1 on, 0 off)

// Control
// int START_V = 3000;
// int PURGE_V = 1000;

extern bool fan_toggle;
extern bool purge_in_process;
extern int purge_step;
extern bool fc_on;

extern uint64_t purge_timer;

void fc_init();
void fc_on_loop();
void fc_purge();
void fc_emergencies();

#endif // FUEL_CELL_CONTROL_H