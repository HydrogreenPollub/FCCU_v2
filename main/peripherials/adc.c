#include "adc.h"
#include "ring_buffer.h"
#include "gpio.h"
// #include"console.h"
// #include "esp_log.h"
// #include "esp_adc/adc_oneshot.h"//????

// *** Calibration coefficients ***
// https://arachnoid.com/polysolve/
float adc_3v3_coeffs[ADC_3V3_VOLTAGE_COEFF_COUNT] = {
     3.3500774500908527e-002,
     7.7284457423388154e-004,
     9.6076736543219649e-008,
    -8.2413978954965527e-011,
     2.7618434159931806e-014,
    -3.3339453919978806e-018
};

float adc_60v_coefficients[ADC_60V_VOLTAGE_COEFF_COUNT] = {
     5.3058557971105280e-001,
     1.3584515496399828e-002,
     4.6881367335382515e-007,
    -2.7055002170240227e-010
};

// CALIBRATION SAMPLES
// filtered raw   real value
// 813 20.5
// 936 27.7
// 969 29.7
// 1015 32.3
// 1072 35.8
// 1100 37.3
// 1180 42.2
// 1236 45.3
// 1269 47.9
// 1281 48.1
// 1355 52.2
// 1362 53.0
// 1418 56.3
float adc_temperature_coeffs[ADC_TEMPERATURE_COEFF_COUNT] = {
    2.0223584704123041e+001,
    2.1664174321726137e-002,
};

float adc_current_flow_coeffs[ADC_CURRENT_FLOW_COEFF_COUNT] = {
    11.9051787046,
    -1.19775806e-002,
    2.442e-007,
};

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_handle_t adc2_handle;

uint16_t buffer_V_FC[ADC_V_FC_SAMPLES_COUNT];
uint16_t buffer_T[ADC_T_SAMPLES_COUNT];
uint16_t buffer_P[ADC_P_SAMPLES_COUNT];
uint16_t buffer_button_state[ADC_BUTTON_SAMPLES_COUNT];
uint16_t buffer_current_flow[ADC_CURRENT_FLOW_SAMPLES_COUNT];
uint16_t buffer_V_SC[ADC_V_SC_SAMPLES_COUNT];
uint16_t buffer_V_USB[ADC_V_USB_SAMPLES_COUNT];

ring_buffer_t rb_V_FC;
ring_buffer_t rb_T;
ring_buffer_t rb_P;
ring_buffer_t rb_button_state;
ring_buffer_t rb_current_flow;
ring_buffer_t rb_V_SC;
ring_buffer_t rb_V_USB;

int V_FC_raw = 0;
float V_FC_filtered_raw = 0;
float V_FC_value = 0;

int T_raw = 0;
float T_filtered_raw = 0;
float T_value = 0;

int P_raw = 0;
float P_filtered_raw = 0;
float P_value = 0;

int button_state_raw = 0;
float button_state_filtered_raw = 0;
float previous_button_state_filtered_raw = 0;
float button_state_value = 0;
float previous_button_state_value = 0;

bool button_state = 0;
bool previous_button_state = 0;

int current_flow_raw = 0;
float current_flow_filtered_raw = 0;
float current_flow_value = 0;

int V_SC_raw = 0;
float V_SC_filtered_raw = 0;
float V_SC_value = 0;

int V_USB_raw = 0;
float V_USB_filtered_raw = 0;
float V_USB_value = 0;

bool led_state = false; // For debugging, toggle LED state

float adc_map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


  float adc_apply_calibration(float coefficients[], uint8_t coeff_count, float adc_raw_sample) //lepiej chybva dac float
{
    // dac swiezakowi jakiemus (minster cyfryzacji prof. nadzwyczajny dr mgr edukacji wczesnoszkolnej inż. Marek K.
    // (podkarpacianianin)(pomnik czynu rewolucyjnego)) z multimetrem zeby spisal adc_raw i napiecie z woltomierza i
    // ulozyc nowe wspolczynniki dla wielomianu na 1 roku jest to to sobie przecwiczy najlepiej tak ze jak juz bedzie
    // szlo do auta albo bedziemy potrzebowac dokladncyh wynikow kalibracje trzeba dla kazdej plytki zrobic ale chyba
    // nie dla adc1 i adc2 oddzielnie

    // TODO filtr cyfrowy dolnoprzepustowy i ring buffer zamiast średniej ruchomej

     float result = 0;//old odwrotne wspolczynniki do narzedzi z sieci
    for (int i = 0; i < coeff_count; i++)
    {
        result += coefficients[i] * pow(adc_raw_sample, i);
    }

    return result;
}  

float adc_apply_filter(ring_buffer_t* rb) // new nie dzieli przez zero
{
    float sum = 0;
    uint16_t sample = 0;
    int valid_samples = 0;

    for (int i = 0; i < rb->count; i++) // tylko tyle, ile jest danych!
    {
        if (ring_buffer_peek(rb, &sample, i))
        {
            sum += sample;
            valid_samples++;
        }
    }

    if (valid_samples == 0)
        return 0; // lub inna wartość domyślna

    return sum / (float) valid_samples;
}

/* float adc_apply_filter(ring_buffer_t* rb) //OLd odwrotnie wspolczynniki
{
    float sum = 0;

    uint16_t sample = 0;
     for (int i = 0; i < rb->size; i++) //old
    //for (int i = 0; i < rb->count; i++)
    {
        if (ring_buffer_peek(rb, &sample, i))
        {
            sum += sample;
        }
    }

    return sum / (float) rb->count;
} */

void adc_on_loop()
{
    // Read raw values from ADC
    adc_oneshot_read(adc1_handle, ADC_V_FC_CHANNEL, &V_FC_raw);
    adc_oneshot_read(adc1_handle, ADC_T_CHANNEL, &T_raw);
    adc_oneshot_read(adc1_handle, ADC_P_CHANNEL, &P_raw);
    adc_oneshot_read(adc2_handle, ADC_BUTTON_STATE_CHANNEL, &button_state_raw);
    adc_oneshot_read(adc1_handle, ADC_CURRENT_FLOW_CHANNEL, &current_flow_raw);
    adc_oneshot_read(adc1_handle, ADC_V_SC_CHANNEL, &V_SC_raw);   // new
    adc_oneshot_read(adc2_handle, ADC_V_USB_CHANNEL, &V_USB_raw); // new
    //  Add new sample to the ring buffer
    ring_buffer_enqueue(&rb_V_FC, V_FC_raw);
    ring_buffer_enqueue(&rb_T, T_raw);
    ring_buffer_enqueue(&rb_P, P_raw);
    ring_buffer_enqueue(&rb_button_state, button_state_raw);
    ring_buffer_enqueue(&rb_current_flow, current_flow_raw);
    ring_buffer_enqueue(&rb_V_SC, V_SC_raw);
    ring_buffer_enqueue(&rb_V_USB, V_USB_raw);

    // Apply filter to the ring buffer
    V_FC_filtered_raw = adc_apply_filter(&rb_V_FC);
    T_filtered_raw = adc_apply_filter(&rb_T);
    P_filtered_raw = adc_apply_filter(&rb_P);
    previous_button_state_filtered_raw = button_state_filtered_raw;
    button_state_filtered_raw = adc_apply_filter(&rb_button_state);
    current_flow_filtered_raw = adc_apply_filter(&rb_current_flow);
    V_SC_filtered_raw = adc_apply_filter(&rb_V_SC);
    V_USB_filtered_raw = adc_apply_filter(&rb_V_USB);

    // Apply calibration curves
   //  V_FC_value = adc_apply_calibration(adc_60v_coefficients, ADC_60V_VOLTAGE_COEFF_COUNT, V_FC_filtered_raw); //old
   V_FC_value  = adc_apply_calibration(adc_60v_coefficients, ADC_60V_VOLTAGE_COEFF_COUNT, V_FC_filtered_raw);

    T_value = adc_apply_calibration(adc_temperature_coeffs, sizeof(ADC_TEMPERATURE_COEFF_COUNT) / sizeof(float), T_filtered_raw);
    // printf("%f\n", T_value);
    P_value = adc_apply_calibration(adc_3v3_coeffs, sizeof(ADC_3V3_VOLTAGE_COEFF_COUNT) / sizeof(float), P_filtered_raw);

    previous_button_state_value = button_state_value;
    // button_state_value = adc_apply_calibration(adc_3v3_coeffs, ADC_3V3_VOLTAGE_COEFF_COUNT,
    // button_state_filtered_raw); current_flow_value = adc_apply_calibration(adc_current_flow_coeffs,
    // ADC_CURRENT_FLOW_COEFF_COUNT, current_flow_filtered_raw);

    V_SC_value = adc_apply_calibration(adc_60v_coefficients, ADC_60V_VOLTAGE_COEFF_COUNT, V_SC_filtered_raw);
    // V_SC_value = V_SC_raw * (55 / 4096.0f);
    V_USB_value = adc_apply_calibration(adc_3v3_coeffs, ADC_3V3_VOLTAGE_COEFF_COUNT, V_USB_filtered_raw);
    V_USB_value = V_USB_value*1.517;
     //V_USB_value = V_USB_filtered_raw * (4.929 / 3718.0f); // Zajebiste

    // V_SC_value = adc_apply_calibration(adc_3v3_coeffs, ADC_3V3_VOLTAGE_COEFF_COUNT, V_FC_filtered_raw);
    // V_SC_value = adc_map(V_SC_value, 0.592, 1.182, 10.0, 20.0);
    // P_value = adc_map(P_value, 0, 2.995, 0, 300); // Pressure: 0-3 V => 0-300 bar
   // ESP_LOGI("ADC", "FC: %3.1f   SC: %4.1f   %3.1f     USB: %2.1f    4.: %2.1f   T: %2.1f", V_FC_value, V_SC_filtered_raw, V_SC_value, V_USB_value, P_value, T_value); // To calibrate

    // ESP_LOGI("ADC", "T_raw=%d T_filtered=%f T_value=%f", T_raw, T_filtered_raw, T_value); // To calibrate

    // To calibrate comment in main console_init();

    gpio_set_level(LED_PIN, !led_state); // Toggle LED state for debugging
    vTaskDelay(50/ portTICK_PERIOD_MS);
}

void adc_init()
{
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    adc_oneshot_new_unit(&init_config2, &adc2_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    // adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config);//IO 0 button
    // adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config);//IO 1
    // adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &config);//IO 2
    // adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config);//IO 3
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config); // IO 4
    // adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_5, &config);//IO 5
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &config);
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_8, &config); // V_FC
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_9, &config); // V_SC
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_0, &config); // V_usb
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_3, &config);

    ring_buffer_init(&rb_V_SC, buffer_V_SC, ADC_V_SC_SAMPLES_COUNT);
    ring_buffer_init(&rb_V_FC, buffer_V_FC, ADC_V_FC_SAMPLES_COUNT);
    ring_buffer_init(&rb_T, buffer_T, ADC_T_SAMPLES_COUNT);
    ring_buffer_init(&rb_P, buffer_P, ADC_P_SAMPLES_COUNT);
    ring_buffer_init(&rb_button_state, buffer_button_state, ADC_BUTTON_SAMPLES_COUNT);
    ring_buffer_init(&rb_V_USB, buffer_V_USB, ADC_V_USB_SAMPLES_COUNT);
    ring_buffer_init(&rb_current_flow, buffer_current_flow, ADC_CURRENT_FLOW_SAMPLES_COUNT);
}
