#include "nwm.h"

uint64_t get_millis()
{
    return (xTaskGetTickCount() * portTICK_PERIOD_MS);
}