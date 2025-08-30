#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "esp_check.h"

// Include our modular components
#include "buttons.h"
#include "led_control.h"
#include "temperature.h"
#include "oled_display.h"
#include "zigbee.h"

static const char *TAG = "AIRTapZB";

// Global state
static int current_fan_speed = 0;

// Button event handler
void buttons_handle_event(button_event_t event) {
    switch (event) {
        case BUTTON_EVENT_UP_PRESS:
            if (current_fan_speed < 10) {
                current_fan_speed++;
                fan_set_speed(current_fan_speed);
                ESP_LOGI(TAG, "Fan speed increased to %d", current_fan_speed);
            }
            break;
            
        case BUTTON_EVENT_DOWN_PRESS:
            if (current_fan_speed > 0) {
                current_fan_speed--;
                fan_set_speed(current_fan_speed);
                ESP_LOGI(TAG, "Fan speed decreased to %d", current_fan_speed);
            }
            break;
            
        case BUTTON_EVENT_TOGGLE_PRESS:
            if (current_fan_speed == 0) {
                // Turn fan on to max speed
                current_fan_speed = 10;
                led_set(true);
                fan_set_speed(current_fan_speed);
                ESP_LOGI(TAG, "Fan toggled ON to speed %d", current_fan_speed);
            } else {
                // Turn fan off
                current_fan_speed = 0;
                led_set(false);
                fan_set_speed(current_fan_speed);
                ESP_LOGI(TAG, "Fan toggled OFF (speed %d)", current_fan_speed);
            }
            break;
            
        case BUTTON_EVENT_TOGGLE_LONG_PRESS:
            ESP_LOGI(TAG, "Factory reset requested");
            zigbee_factory_reset();
            break;
            
        case BUTTON_EVENT_MODE_PRESS:
            ESP_LOGI(TAG, "Pairing mode requested");
            if (!pairing_mode_active) {
                ESP_LOGI(TAG, "Starting pairing mode");
                zigbee_start_pairing();
            } else {
                ESP_LOGI(TAG, "Cancelling pairing mode");
                zigbee_cancel_pairing();
            }
            break;
            
        default:
            break;
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting AirTap T-Series with Zigbee");
    
    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    
    // Initialize all components
    buttons_init();
    led_control_init();
    temperature_init();
    oled_init();
    zigbee_init();
    
    // Create Zigbee task
    xTaskCreate(zigbee_task, "Zigbee_main", 4096, NULL, 5, NULL);
    
    // Main loop
    uint32_t last_update = 0;
    while (true) {
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        
        // Update display every second
        if (now - last_update >= 1000) {
            last_update = now;
            
            // Read temperature
            float temp_c = temperature_read_celsius();
            
            // Update display
            uint32_t uptime_seconds = (uint32_t)(esp_timer_get_time() / 1000000);
            oled_update_display(temp_c, current_fan_speed, pairing_mode_active, 
                              factory_reset_pending, zb_joined, uptime_seconds);
        }
        
        // Scan buttons and handle events
        button_event_t event = buttons_scan();
        if (event != BUTTON_EVENT_NONE) {
            buttons_handle_event(event);
        }
        
        // Small delay
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
