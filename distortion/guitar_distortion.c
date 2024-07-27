#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"  // Include header for clock functions
#include <math.h>

// Constants
#define ADC_PIN 26          // GPIO26 (ADC0)
#define PWM_PIN 18          // GPIO18 (PWM)
#define SAMPLE_RATE 8000    // 8 kHz sample rate
#define MAX_ADC_VALUE 4095  // 12-bit ADC
#define MAX_PWM_VALUE 65535 // 16-bit PWM

// Initialize PWM
void setup_pwm() {
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_wrap(slice_num, MAX_PWM_VALUE);
    pwm_set_clkdiv(slice_num, (float)clock_get_hz(clk_sys) / (SAMPLE_RATE * MAX_PWM_VALUE));
    pwm_set_enabled(slice_num, true);
}

// Apply distortion effect
uint16_t apply_distortion(uint16_t sample) {
    // Example distortion effect: simple clipping
    int16_t centered_sample = sample - (MAX_ADC_VALUE / 2);
    int16_t clipped_sample = centered_sample > 2048 ? 2048 : (centered_sample < -2048 ? -2048 : centered_sample);
    return (clipped_sample + (MAX_ADC_VALUE / 2)) * (MAX_PWM_VALUE / MAX_ADC_VALUE);
}

int main() {
    stdio_init_all();

    // Initialize ADC
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    setup_pwm();

    uint16_t sample;
    while (1) {
        // Read ADC
        sample = adc_read();

        // Apply distortion
        uint16_t distorted_sample = apply_distortion(sample);

        // Output distorted sample via PWM
        pwm_set_gpio_level(PWM_PIN, distorted_sample);

        sleep_us(1000000 / SAMPLE_RATE);
    }

    return 0;
}
