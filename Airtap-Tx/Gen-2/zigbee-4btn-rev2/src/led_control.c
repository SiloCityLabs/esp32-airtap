#include "led_control.h"

static const char *TAG = "LED_CONTROL";

void led_control_init(void) {
    // Initialize GPIO for LED
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << PIN_LED),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "LED control initialized");
}

void led_set(bool state) {
    gpio_set_level(PIN_LED, !state); // Invert the state for correct LED behavior
}
