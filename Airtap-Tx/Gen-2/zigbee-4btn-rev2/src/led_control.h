#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

// Pin definitions
#define PIN_PWM_FAN     0
#define PIN_LED         15  // Built-in LED on XIAO ESP32C6

// Function prototypes
void led_control_init(void);
void led_set(bool state);
void fan_set_speed(int speed);
void fan_apply_pwm(void);

#endif // LED_CONTROL_H
