#include "temperature.h"
#include <math.h>

static const char *TAG = "TEMPERATURE";

// ADC handles
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle = NULL;
static bool adc_calibration_init_done = false;

void temperature_init(void) {
    // Initialize ADC for temperature
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));
    
    ESP_LOGI(TAG, "Temperature sensor initialized");
}

int16_t temperature_read_centi(void) {
    int adc_raw;
    int voltage;
    esp_err_t ret = adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_raw);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error reading ADC");
        return 2200; // Return 22.0°C as default
    }
    
    if (adc_calibration_init_done) {
        ret = adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw, &voltage);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error converting ADC to voltage");
            return 2200;
        }
    } else {
        voltage = adc_raw;
    }
    
    // Convert voltage to temperature (NTC thermistor)
    // This is a simplified conversion - you may need to calibrate for your specific thermistor
    float voltage_ratio = (float)voltage / 3300.0f; // Assuming 3.3V reference
    float resistance = 10000.0f * voltage_ratio / (1.0f - voltage_ratio); // 10k pull-up
    float temp_k = 1.0f / (1.0f/298.15f + 1.0f/3950.0f * log(resistance/10000.0f)); // B=3950K
    float temp_c = temp_k - 273.15f;
    
    return (int16_t)(temp_c * 100.0f);
}

float temperature_read_celsius(void) {
    return (float)temperature_read_centi() / 100.0f;
}

float temperature_read_fahrenheit(void) {
    float temp_c = temperature_read_celsius();
    return temp_c * 9.0f / 5.0f + 32.0f;
}
