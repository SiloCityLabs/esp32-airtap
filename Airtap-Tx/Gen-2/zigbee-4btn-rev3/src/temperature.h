#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_log.h"

// Pin definitions
#define PIN_ADC_TEMP    2

// Function prototypes
void temperature_init(void);
int16_t temperature_read_centi(void);
float temperature_read_celsius(void);
float temperature_read_fahrenheit(void);

#endif // TEMPERATURE_H
