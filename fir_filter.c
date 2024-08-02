#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include <math.h>

#define SAMPLE_RATE 44100
#define ADC_PIN 26
#define PWM_PIN 18
#define FILTER_ORDER 3

// FIR Filter Coefficients for Low-Pass Filter

float coefficients[FILTER_ORDER] = {0.3333, 0.3333, 0.3333};

// Input buffer
float buffer[FILTER_ORDER] = {0.0};

// Initialize the ADC

void init_adc() {
  adc_init();
  adc_gpio_init(ADC_PIN);
  adc_select_input(0);
}

// Initialize the PWM

void init_pwm() {
  gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
  pwm_set_wrap(slice_num, 255);
  pwm_set_clkdiv(slice_num, 4.0f); // sys clock 125 MHz
  pwm_set_enabled(slice_num, true);
}

// get sample from the ADC

uint16_t get_adc_sample() {
  adc_run(true);
  sleep_us(2); // small delay for ADC to stabilize
  unint16_t sample = adc_read();
  adc_run(false);
  return sample;
}

// set sample to PWM

void set_pwm_sample(uint16_t sample){
  uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
  pwm_set_gpio_level(PWM_PIN, sample);  
}

// FIR Filter Function

float fir_filter(float input){
  // Shift buffer values
  for (int i = FILTER_ORDER - 1; i > 0; i--){
      buffer[i] = buffer[i - 1];
  }
  buffer[0] = input;

  // Apply filter coefficients
  float output = 0.0;
  for (int i = 0; i < FILTER_ORDER; i++){
    output += coefficients[i] * buffer[i];
  }

  return output;
}

int main() {
  stdio_init_all();
  init_adc();
  init_pwm();

  while(true){
    uint16_t adc_value = get_adc_sample();
    float input = ((float)adc_value / 4095.0f) * 2.0f - 1.0f; // Convert ADC value to -1.0 to 1.0
    float filtered_sample = fir_filter(input);
    uint16_t output_value = (uint16_t)((filtered_sample + 1.0f) * 127.5f); // Convert back to 0-255 for PWM
    set_pwm_sample(output_value);
    sleep_us(1000000 / SAMPLE_RATE); // Maintain the sample rate
  }
  return 0;
}
