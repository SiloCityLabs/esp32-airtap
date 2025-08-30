#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include "driver/ledc.h"
#include "esp_log.h"

// Pin definitions
#define PIN_PWM_FAN     0

// Function prototypes
void fan_control_init(void);
void fan_set_speed(int speed);
void fan_apply_pwm(void);
int fan_get_speed(void);

#endif // FAN_CONTROL_H
