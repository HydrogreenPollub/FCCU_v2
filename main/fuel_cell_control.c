#include "fuel_cell_control.h"
#include "adc.h"
#include "pwm.h"
#include "gpio.h"
#include "timer.h"

// Start value
bool main_valve = 0;  // ================= ustawienie zaworu zasilajacego _ main valve n-mosfet (1 on, 0 off)
bool purge_valve = 0; // ================= ustawienie przedmuchu - purge valve n-mosfet (1 on, 0 off)
bool mosfet = 0;      // ================= ustawienie zwarcia mosfet - driver i n-mosfet (1 on, 0 off)

// Control
// int START_V = 3000;
// int PURGE_V = 1000;

bool fan_toggle = 0;
bool purge_in_process = 0;
int purge_step = 0;

uint64_t purge_timer;

void fc_purge()
{
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
                    pwm_set_current_control_duty_cycle(0);
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
                pwm_set_current_control_duty_cycle(100);
                break;
        }
    }
    /*if (purge_step == 1 && purge_timer > PURGE_DURATION_MS)
    {
        gpio_set_level(PURGE_VALVE_PIN, 0);
        purge_step++;
        purge_timer = get_millis();
        // return;
    }*/

    /* if (purge_step == 2 && purge_timer > PURGE_MOSFET_DELAY_MS)
     {
         gpio_set_level(PURGE_VALVE_PIN, 0);
         gpio_set_level(MOSFET_PIN, 1);
         purge_step++;
         purge_timer = get_millis();
         // return;
     }*/

    /*if (purge_step == 3 && purge_timer > MOSFET_SHORT_DURATION_MS)
    {
        gpio_set_level(MOSFET_PIN, 0);
        purge_step++;
        purge_timer = get_millis();
    }*/

    /*if (purge_step == 4 && purge_timer > MOSFET_SHORT_INTERVAL_MS)
    {
        gpio_set_level(MOSFET_PIN, 0);
        purge_step++;
        purge_timer = get_millis();
    }

    if (purge_step == 5 && purge_timer > MOSFET_SHORT_DURATION_MS)
    {
        gpio_set_level(MOSFET_PIN, 0);
        purge_step++;
        purge_timer = get_millis();
    }

    if (purge_step == 6 && purge_timer > MOSFET_SHORT_INTERVAL_MS)
    {
        gpio_set_level(MOSFET_PIN, 0);
        purge_step++;
        purge_timer = get_millis();
    }

    if (purge_step == 7 && purge_timer > MOSFET_SHORT_DURATION_MS)
    {
        gpio_set_level(MOSFET_PIN, 0);
        gpio_set_level(PURGE_VALVE_PIN, 0);
        purge_step = 0;
        purge_in_process = 0;
    }
}
else
{
    purge_valve = 0;
    mosfet = 0;
}*/
}

void fc_on_loop()
{
    /*TEST*/ /*main_valve = 1;
    purge_valve = 1;
    fan_gnd_duty_cycle_percent = 1;
    fan_PWM_duty_cycle_percent = 30; // testy 3s ????*/

    // fc_on_loop(); *************************

    // Fan gnd control
    // pwm_set_gnd_duty_cycle(fan_gnd_duty_cycle_percent);

    /*Fan_PWM_control*/ // pwm_set_pwm_duty_cycle(fan_PWM_duty_cycle_percent);
    // TEST end

    // przestarzałe:
    //  How to control PEM fuel cell?
    //  power main valve, power fans, measure voltage, measure temperature,
    //  if temperature is rising from 40*C give higher PWM proportional to temp, if maksimum temp 65*C turn off fuel
    //  cell and give flag if voltage falling down in time about 1 min turn purge valve and mosfet on about 1ms , if
    //  voltage to low 15V turn off and give flag

    if (button_state_value /*_average*/ > 2.9 && previous_button_state_value < 2.9)
    {
        fan_toggle = !fan_toggle;
        if (fan_toggle == 0)
        {
            main_valve = 0;
            fan_gnd_duty_cycle_percent = 0;
            fan_PWM_duty_cycle_percent = 0;
            printf("main off\n");
            gpio_set_level(MAIN_VALVE_PIN, 0);
        }
        else
        {
            main_valve = 1;
            fan_gnd_duty_cycle_percent = 100;
            fan_PWM_duty_cycle_percent = 100;
            gpio_set_level(MAIN_VALVE_PIN, 1);
            printf("main on\n");
        }
        // fan_toggle = !fan_toggle;
    }
    else
    {
        // Przedmuchy
        if (button_state_value > 0.9 && button_state_value < 1.2 && (previous_button_state_value < 0.9))
        {
            purge_in_process = 1;
        }
    }

    if (fan_toggle == 1)
    {
        if (T_value > 55)
        {
            fan_PWM_duty_cycle_percent = (int) (T_value * 1.5);
        }
        else if (T_value > 20)
        {
            fan_PWM_duty_cycle_percent = T_value - 10; // T_value /*_average - (-30) /*10; // If you have simple 2 wire
        }
        else
        {
            fan_PWM_duty_cycle_percent = 0;
        }

        pwm_set_pwm_duty_cycle(fan_PWM_duty_cycle_percent);
        pwm_set_gnd_duty_cycle(fan_gnd_duty_cycle_percent);
    }
    pwm_set_gnd_duty_cycle(50);

    /* if (T_value > 70)
     {
         fan_gnd_duty_cycle_percent = 0;
         fan_PWM_duty_cycle_percent = 0;
         gpio_set_level(MAIN_VALVE_PIN, 0);
         gpio_set_level(PURGE_VALVE_PIN, 0);
         gpio_set_level(MOSFET_PIN, 0);
     }*/

    fc_purge();
}