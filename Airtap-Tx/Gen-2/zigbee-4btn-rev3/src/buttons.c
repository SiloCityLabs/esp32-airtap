#include "buttons.h"

static const char *TAG = "BUTTONS";

// Button scan state
static uint32_t last_press_time = 0;
static uint32_t toggle_press_start = 0;
static bool toggle_pressed = false;
static bool mode_prev_pressed = false;

void buttons_init(void) {
    // Configure buttons as inputs with pull-up
    gpio_config_t btn_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << PIN_BTN_MODE) | (1ULL << PIN_BTN_UP) | (1ULL << PIN_BTN_DOWN) | (1ULL << PIN_BTN_TOGGLE),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&btn_conf);
    ESP_LOGI(TAG, "Buttons initialized");
}

button_event_t buttons_scan(void) {
    uint32_t current_time = (uint32_t)(esp_timer_get_time() / 1000);
    
    // Debounce time
    if (current_time - last_press_time < 200) {
        return BUTTON_EVENT_NONE;
    }
    
    // UP button
    if (gpio_get_level(PIN_BTN_UP) == 0) {
        last_press_time = current_time;
        return BUTTON_EVENT_UP_PRESS;
    }
    
    // DOWN button
    if (gpio_get_level(PIN_BTN_DOWN) == 0) {
        last_press_time = current_time;
        return BUTTON_EVENT_DOWN_PRESS;
    }
    
    // Toggle button with long-press detection for factory reset
    if (gpio_get_level(PIN_BTN_TOGGLE) == 0) {
        if (!toggle_pressed) {
            toggle_pressed = true;
            toggle_press_start = current_time;
        } else {
            // Check for long press (3 seconds)
            if (current_time - toggle_press_start >= 3000) {
                toggle_pressed = false;
                last_press_time = current_time;
                return BUTTON_EVENT_TOGGLE_LONG_PRESS;
            }
        }
    } else {
        if (toggle_pressed) {
            // Short press - toggle fan
            if (current_time - toggle_press_start < 3000) {
                toggle_pressed = false;
                last_press_time = current_time;
                return BUTTON_EVENT_TOGGLE_PRESS;
            }
            toggle_pressed = false;
            last_press_time = current_time;
        }
    }
    
    // MODE button: start pairing on falling edge and keep it active until Zigbee finishes
    bool mode_pressed = (gpio_get_level(PIN_BTN_MODE) == 0);
    if (mode_pressed && !mode_prev_pressed) {
        last_press_time = current_time;
        mode_prev_pressed = mode_pressed;
        return BUTTON_EVENT_MODE_PRESS;
    }
    mode_prev_pressed = mode_pressed;
    
    return BUTTON_EVENT_NONE;
}
