#include "led_control.h"

static const char *TAG = "LED_CONTROL";

// Global fan speed state
static int current_fan_speed = 0;

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
    
    // Initialize PWM for fan control
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 25000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);
    
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = PIN_PWM_FAN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0,
    };
    ledc_channel_config(&ledc_channel);
    
    ESP_LOGI(TAG, "LED and PWM control initialized");
}

void led_set(bool state) {
    gpio_set_level(PIN_LED, state);
}

void fan_set_speed(int speed) {
    if (speed < 0) speed = 0;
    if (speed > 10) speed = 10;
    current_fan_speed = speed;
    fan_apply_pwm();
}

void fan_apply_pwm(void) {
    if (current_fan_speed == 0) {
        ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    } else {
        uint32_t duty = (current_fan_speed * 8191) / 10; // 8191 is max duty for 13-bit resolution
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
}

int fan_get_speed(void) {
    return current_fan_speed;
}
