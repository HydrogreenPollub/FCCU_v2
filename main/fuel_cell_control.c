#include "fuel_cell_control.h"
#include "adc.h"
#include "pwm.h"
#include "gpio.h"
#include "timer.h"

// Start value
bool main_valve_toggle = 0; // ================= ustawienie zaworu zasilajacego _ main valve n-mosfet (1 on, 0 off)
bool purge_valve = 0;       // ================= ustawienie przedmuchu - purge valve n-mosfet (1 on, 0 off)
bool mosfet_toggle = 0;     // ================= ustawienie zwarcia mosfet - driver i n-mosfet (1 on, 0 off)
bool fc_on = 0;
bool fan_toggle_trigger = 0;

float FC_V_buffer[FC_V_PROBING_TIME * FC_V_PROBING_FREQUENCY] = { 0 };
uint64_t FC_V_probing_timer;
int FC_V_buffer_current_idx = 0;

bool fan_toggle = 0;
bool purge_in_process = 0;
int purge_step = 0;

uint64_t purge_timer;

void fc_init()
{
    for (int i = 0; i < FC_V_PROBING_TIME * FC_V_PROBING_FREQUENCY; i++)
    {
        FC_V_buffer[i] = 0;
    }
    purge_timer = get_millis();
    FC_V_probing_timer = get_millis();
}

void fc_purge()
{
    if (get_millis() - FC_V_probing_timer > 1000.0 / FC_V_PROBING_FREQUENCY)
    {
        FC_V_buffer[FC_V_buffer_current_idx] = V_FC_value;
        FC_V_buffer_current_idx++;
        FC_V_buffer_current_idx %= (FC_V_PROBING_FREQUENCY * FC_V_PROBING_TIME);
        FC_V_probing_timer = get_millis();
    }

    if (purge_in_process)
    {
        switch (purge_step)
        {
            case 0:
                gpio_set_level(PURGE_VALVE_PIN, 1);
                printf("%lld\t%d\n", get_millis(), purge_step);
                purge_step++;
                purge_timer = get_millis();
                break;
            case 1:
                if (get_millis() - purge_timer >= PURGE_DURATION_MS)
                {
                    printf("%lld\t%d\n", get_millis(), purge_step);
                    gpio_set_level(PURGE_VALVE_PIN, 0);
                    purge_step++;
                    purge_timer = get_millis();
                    // return;
                }
                break;
            case 2:
                if (get_millis() - purge_timer >= PURGE_MOSFET_DELAY_MS)
                {
                    printf("%lld\t%d\n", get_millis(), purge_step);
                    gpio_set_level(PURGE_VALVE_PIN, 0);
                    gpio_set_level(MOSFET_PIN, 1);
                    purge_step++;
                    purge_timer = get_millis();
                    pwm_set_current_control_duty_cycle(0); // possibly deprecated
                    // return;
                }
                break;
            case 3:
            case 5:
            case 7:
                if (get_millis() - purge_timer >= MOSFET_SHORT_DURATION_MS)
                {
                    printf("%lld\t%d\n", get_millis(), purge_step);
                    gpio_set_level(MOSFET_PIN, 0);
                    purge_step++;
                    purge_timer = get_millis();
                }
                break;
            case 4:
            case 6:
                if (get_millis() - purge_timer >= MOSFET_SHORT_INTERVAL_MS)
                {
                    printf("%lld\t%d\n", get_millis(), purge_step);
                    gpio_set_level(MOSFET_PIN, 1);
                    purge_step++;
                    purge_timer = get_millis();
                }
                break;
            default:
                printf("%lld\t%d\n", get_millis(), purge_step);
                gpio_set_level(MOSFET_PIN, 0);
                gpio_set_level(PURGE_VALVE_PIN, 0);
                purge_step = 0;
                purge_in_process = 0;
                pwm_set_current_control_duty_cycle(100); // possibly deprecated
                break;
        }
    }
    else
    {
        if (FC_V_buffer[(FC_V_buffer_current_idx - 1 + FC_V_PROBING_FREQUENCY * FC_V_PROBING_TIME)
                        % FC_V_PROBING_FREQUENCY * FC_V_PROBING_TIME]
                - FC_V_buffer[FC_V_buffer_current_idx]
            >= FC_V_PURGE_TRIGGER_DIFFERENCE)
        {
            purge_in_process = 1;
        }
    }
}

void fc_on_loop()
{
    // startup
    if (!fc_on && gpio_get_level(EMERGENCY_BUTTON_PIN) == 0)
    {
        fan_toggle_trigger = 1;
        pwm_set_pwm_duty_cycle(100);
        pwm_set_gnd_duty_cycle(100);
        vTaskDelay(700 / portTICK_PERIOD_MS); // TODO: make a timer
        gpio_set_level(MAIN_VALVE_PIN, 1);
        fc_on = 1;
    }

    // emergency button press
    if (fc_on && gpio_get_level(EMERGENCY_BUTTON_PIN) == 1)
    {
        pwm_set_pwm_duty_cycle(100);
        pwm_set_gnd_duty_cycle(100);
        gpio_set_level(MAIN_VALVE_PIN, 0);
        while (V_FC_value > 3.0)
            ; // TODO: find correct value
        pwm_set_pwm_duty_cycle(0);
        pwm_set_gnd_duty_cycle(0);
        fc_on = 0;
        fan_toggle_trigger = 1;
    }

    // supercapacitor or fuel cell emergency
    float SC_V_value = 48.0; // TODO: implement reading and handling SC_V
    if (SC_V_value < SC_MINIMAL_VOLTAGE || V_FC_value < FC_MINIMAL_VOLTAGE)
    {
        gpio_set_level(MAIN_VALVE_PIN, 0);
        pwm_set_pwm_duty_cycle(100);
        pwm_set_gnd_duty_cycle(100);
        while (V_FC_value > 3.0)
            ; // TODO: find correct value
        pwm_set_pwm_duty_cycle(0);
        pwm_set_gnd_duty_cycle(0);
        fc_on = 0;
        fan_toggle = 0;
    }

    // probably deprecated
    /*if (fc_on && fan_toggle_trigger button_state_value > 2.9 && previous_button_state_value < 2.9)
     {
         fan_toggle_trigger = 0;
         fan_toggle = !fan_toggle;
         if (fan_toggle == 0)
         {
             main_valve_toggle = 0;
             fan_gnd_duty_cycle_percent = 0;
             fan_PWM_duty_cycle_percent = 0;
             gpio_set_level(MAIN_VALVE_PIN, 0);
         }
         else
         {
             main_valve_toggle = 1;
             fan_gnd_duty_cycle_percent = 100;
             fan_PWM_duty_cycle_percent = 100;
             gpio_set_level(MAIN_VALVE_PIN, 1);
         }
     }
     else
     {
         // Force purge trigger
         if (button_state_value > 0.9 && button_state_value < 1.2 && (previous_button_state_value < 0.9))
         {
             purge_in_process = 1;
         }
         // deprecated??
     }
     // fan_toggle = 1;*/

    if (fan_toggle == 1)
    {
        if (T_value > 55)
        {
            fan_PWM_duty_cycle_percent = (int) (T_value * 1.5);
        }
        else if (T_value > 40)
        {
            fan_PWM_duty_cycle_percent = (int) (T_value - 10);
        }
        else
        {
            fan_PWM_duty_cycle_percent = 0;
        }
    }
    pwm_set_pwm_duty_cycle((int) fan_toggle * fan_PWM_duty_cycle_percent);
    pwm_set_gnd_duty_cycle((int) fan_toggle * fan_gnd_duty_cycle_percent);

    fc_purge();
}