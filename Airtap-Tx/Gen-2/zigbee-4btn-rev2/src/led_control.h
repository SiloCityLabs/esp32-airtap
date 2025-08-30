#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "driver/gpio.h"
#include "esp_log.h"

// Pin definitions
#define PIN_LED         15  // Built-in LED on XIAO ESP32C6

// Function prototypes
void led_control_init(void);
void led_set(bool state);

#endif // LED_CONTROL_H
