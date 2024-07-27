#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "math.h"

#define SAMPLE_RATE 44100
#define LFO_FREQ 0.5
#define DEPTH 0.5
#define FEEDBACK 0.7
#define MAX_DELAY 0.002 // 2ms max delay
#define BUFFER_SIZE (SAMPLE_RATE * MAX_DELAY)
#define ADC_PIN 26          // GPIO26 (ADC0)
#define PWM_PIN 18          // GPIO18 (PWM)

float lfo_phase = 0.0f;
float lfo_increment;
float delay_buffer(BUFFER_SIZE);
int buffer_index = 0;

// Initialize ADC and PWM

void init_adc(){
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);
}

void init_pwm(){
    gpio_set_function(28, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(28);
    pwm_set_wrap(slice_num, 255);
    pwm_set_gpio_level(28, 0);
    pwm_set_enabled(slice_num, true);
}

// Get sample from the ADC

unit16_t get_adc_sample() {
    adc_fifo_drain();
    adc_run(true);
    while (adc_fifo_is_empty()){
        tight_loop_contenst();
    }
    uint16_t sample = adc_fifo.get();
    adc_run(false);
    return sample;
}


// Set sample to PWM

void set_pwm_sample(unit16_t sample){
    uint slice_num = pwm_gpio_to_slice_num(28);
    pwm_set_gpio_level(28, sample);
}

// Phaser effect signal processing

float process_phaser(float input){
    lfo_phase += lfo_increment;
    if (lfo_phase >= 1.0f) lfo_phase -= 1.0f;

    float lfo_value = sinf(lfo_phase * 2.0f * M_PI);
    int delay_samples = (int)((1.0f + lfo_value * DEPTH) * (BUFFER_SIZE / 2));

    int read_index = buffer_index - delay_samples;
    if (read_index < 0) read_index += BUFFER_SIZE;

    float delayed_sample = delay_buffer[read_index];
    delay_buffer[buffer_index] = input + delayed_sample * FEEDBACK;

    buffer_index++;
    if (buffer_index >= BUFFER_SIZE) buffer_index = 0;

    return input + delayed_sample * 0.7;
}

int main() {
    stdio_init_all();
    init_adc();
    init_pwm();

    lfo_increment = LFQ_FREQ / SAMPLE_RATE;

    while(true) {
        unit16_t adc_value = get_adc_sample();
        float input = ((float)adc_value / 4095.0f) * 2.0f - 1.0f; // Convert ADC value to -1.0 to 1.0
        float processed_sample = process_phaser(input);

        uint16_t output_value = (unit16_t)((processed_sample + 1.0f) * 127.5f);// Convert back to 0-255 for PWM
        
        set_pwm_sample(output_value);
    }
}