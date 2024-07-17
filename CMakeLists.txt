cmake_minimum_required(VERSION 3.13)

include(C:/Pico/pico-sdk/pico_sdk_init.cmake)

project(guitar_distortion C CXX ASM)

pico_sdk_init()

add_executable(audio_distortion
    guitar_distortion.c
)

target_link_libraries(audio_distortion pico_stdlib hardware_adc hardware_pwm)

pico_add_extra_outputs(audio_distortion)
