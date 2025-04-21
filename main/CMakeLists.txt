idf_component_register(
    SRCS "peripherials/can.c" "peripherials/console.c" "peripherials/gpio.c" "peripherials/pwm.c" "peripherials/adc.c" "peripherials/timer.c" "peripherials/uart.c" "fuel_cell_control.c"
         "utils/ring_buffer.c" 
         "main.c"
    INCLUDE_DIRS "." "peripherials" "utils"
)
