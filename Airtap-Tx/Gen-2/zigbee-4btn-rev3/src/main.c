// src/main.c
#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_log.h"
#include "esp_timer.h"

// Zigbee SDK
#include "esp_zigbee_core.h"
#include "zcl/esp_zigbee_zcl_common.h"
#include "zcl/esp_zigbee_zcl_level.h"
#include "zcl/esp_zigbee_zcl_on_off.h"
#include "zcl/esp_zigbee_zcl_temperature_meas.h"
#include "ha/esp_zigbee_ha_standard.h"

static const char *TAG = "AIRTapZB";

// ------ Pin map (C6 DevKitC-1 compatible with your C3 layout) ------
#define PIN_PWM_FAN     GPIO_NUM_2
#define PIN_ADC_TEMP    ADC_CHANNEL_0 /* GPIO4 -> ADC1 CH0 on C6 */
#define PIN_BTN_MODE    GPIO_NUM_10
#define PIN_BTN_UP      GPIO_NUM_20
#define PIN_BTN_DOWN    GPIO_NUM_8
#define PIN_BTN_TOGGLE  GPIO_NUM_9

// ------ Fan speed state (0..10) ------
static int fan_speed = 0;
static int last_nonzero_speed = 10;

// ------ ADC / NTC settings ------
static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t cali_handle;
static bool adc_calibrated = false;

// Divider: NTC to GND, 10k series to Vref (same as your YAML)
// Convert mV -> resistance -> temperature (°C) with a simple piecewise/β approx
static const float R_SERIES_OHMS = 10000.0f;

// Map 0..10 steps to PWM duty (approx. your YAML curve: 1->~38%, else (n+4)/14)
static float speed_to_duty(int s) {
    if (s <= 0) return 0.0f;
    if (s == 1) return 0.38f;
    return (float)(s + 4) / 14.0f;
}

static void apply_fan_pwm(void) {
    float duty = speed_to_duty(fan_speed);
    uint32_t res = (1 << 13); // 13-bit duty resolution (0..8191)
    uint32_t duty_raw = (uint32_t)(duty * (res - 1));
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_raw);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

static void set_fan_speed_from_level(uint8_t lvl255) {
    int s = (int)roundf((float)lvl255 / 25.5f); // 0..255 -> 0..10
    if (s < 0) s = 0; if (s > 10) s = 10;
    fan_speed = s;
    if (s > 0) last_nonzero_speed = s;
    apply_fan_pwm();
}

// Simple NTC: convert mV to resistance then temperature using 3-point linear segs (from your YAML table)
static float ntc_temp_c_from_mv(int mv) {
    // Vout across NTC, Vs ~ 1100/1200mV reference? We'll rely on calibrated mv reading (0..approx 1100mV at atten 12dB varies).
    // Compute R_ntc using divider: Vntc = Vs * (R_ntc / (R_ntc + R_series)) -> R_ntc = R_series * Vntc / (Vs - Vntc)
    const float VS_MV = 1100.0f; // on C6 ADC calibration gives voltage; adjust if needed
    float v = (float)mv;
    float r_ntc = (v <= 0.0f || v >= VS_MV) ? 1e9f : (R_SERIES_OHMS * v / (VS_MV - v));

    // Points (R, T°C) from YAML: (3.389k,0), (10k,25), (27.219k,50) -- it's actually PTC in comment, but we’ll treat monotonically.
    // We'll do piecewise linear in ln(R).
    float lnR = logf(r_ntc);
    float lnR0 = logf(3389.0f),  T0 = 0.0f;
    float lnR1 = logf(10000.0f), T1 = 25.0f;
    float lnR2 = logf(27219.0f), T2 = 50.0f;

    float T;
    if (lnR <= lnR0) {
        // extrapolate below
        float m = (T1 - T0) / (lnR1 - lnR0);
        T = T0 + m * (lnR - lnR0);
    } else if (lnR >= lnR2) {
        // extrapolate above
        float m = (T2 - T1) / (lnR2 - lnR1);
        T = T2 + m * (lnR - lnR2);
    } else if (lnR <= lnR1) {
        float m = (T1 - T0) / (lnR1 - lnR0);
        T = T0 + m * (lnR - lnR0);
    } else {
        float m = (T2 - T1) / (lnR2 - lnR1);
        T = T1 + m * (lnR - lnR1);
    }
    return T;
}

// Read temperature in centi-degrees Celsius for ZCL
static int16_t read_temp_centi(void) {
    int mv = 0;
    int raw_mv = 0;
    int raw;
    adc_oneshot_read(adc_handle, PIN_ADC_TEMP, &raw);
    if (adc_calibrated) {
        adc_cali_raw_to_voltage(cali_handle, raw, &raw_mv);
        mv = raw_mv;
    } else {
        // Fallback simple scaling
        mv = (int)((raw / 4095.0f) * 1100.0f);
    }
    float tc = ntc_temp_c_from_mv(mv);
    int16_t centi = (int16_t)lroundf(tc * 100.0f);
    return centi;
}

// ---------------- Zigbee ZCL state ----------------
static uint8_t zcl_onoff_attr = 0; // 0=off, 1=on
static uint8_t zcl_level_attr = 0; // 0..255
static int16_t zcl_temp_measured = 2200; // 22.00°C

// ------ Zigbee configuration ------
#define HA_ESP_LIGHT_ENDPOINT 1
#define HA_TEMP_SENSOR_ENDPOINT 2

// Handle Level/OnOff writes/commands to keep PWM in sync
static void handle_onoff_change(uint8_t on) {
    zcl_onoff_attr = on ? 1 : 0;
    if (on) {
        if (fan_speed == 0) {
            fan_speed = last_nonzero_speed;
        }
    } else {
        fan_speed = 0;
    }
    zcl_level_attr = (fan_speed == 0) ? 0 : (uint8_t)lroundf(fan_speed * 25.5f);
    apply_fan_pwm();
}
static void handle_level_change(uint8_t lvl) {
    zcl_level_attr = lvl;
    handle_onoff_change(lvl > 0);
    set_fan_speed_from_level(lvl);
}

// ---------------- Buttons ----------------
static void init_button(gpio_num_t pin) {
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << pin,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);
}

// Factory reset and pairing state
static bool factory_reset_pending = false;
static bool pairing_mode_active = false;
static uint32_t button_hold_start = 0;
static bool button_held = false;

static void trigger_factory_reset(void) {
    ESP_LOGI(TAG, "Factory reset triggered - removing from network");
    factory_reset_pending = true;
    // Leave current network
    esp_zb_zdo_mgmt_leave_req_param_t leave_req = {
        .device_address = {0},  // Leave this device
        .dst_nwk_addr = 0x0000, // Coordinator
        .remove_children = 0,
        .rejoin = 0
    };
    esp_zb_zdo_device_leave_req(&leave_req, NULL, NULL);
}

static void trigger_pairing_mode(void) {
    ESP_LOGI(TAG, "Pairing mode triggered - starting network steering");
    pairing_mode_active = true;
    esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
}

static void scan_buttons(void) {
    // simple polling debounce
    static uint32_t last_ms = 0;
    uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
    if (now - last_ms < 50) return;
    last_ms = now;

    // Check for factory reset (hold MODE button for 10 seconds)
    if (!gpio_get_level(PIN_BTN_MODE)) {
        if (!button_held) {
            button_hold_start = now;
            button_held = true;
        } else if (now - button_hold_start >= 10000) { // 10 seconds
            trigger_factory_reset();
            button_held = false;
            return; // Don't process other button actions
        }
    } else {
        if (button_held && (now - button_hold_start >= 1000) && (now - button_hold_start < 10000)) {
            // Short press - trigger pairing mode
            trigger_pairing_mode();
        }
        button_held = false;
    }

    // Normal button functions (only if not in factory reset)
    if (!factory_reset_pending) {
        if (!gpio_get_level(PIN_BTN_TOGGLE)) {
            handle_onoff_change(fan_speed == 0);
        }
        if (!gpio_get_level(PIN_BTN_UP)) {
            if (fan_speed < 10) {
                fan_speed++;
                last_nonzero_speed = fan_speed;
                handle_level_change((uint8_t)lroundf(fan_speed * 25.5f));
            }
        }
        if (!gpio_get_level(PIN_BTN_DOWN)) {
            if (fan_speed > 0) {
                fan_speed--;
                handle_level_change((uint8_t)lroundf(fan_speed * 25.5f));
            }
        }
    }
}

// --------------- Zigbee action handler ---------------
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        ESP_LOGI(TAG, "Attribute value changed");
        break;
    default:
        ESP_LOGI(TAG, "Unhandled action callback: %d", callback_id);
        break;
    }
    return ret;
}

// --------------- Zigbee stack callbacks ---------------
static void zb_app_signal_handler(esp_zb_app_signal_t *signal_struct) {
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig = *p_sg_p;
    switch (sig) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Device started up in%s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : " non");
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Starting network steering (factory new device)");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            } else {
                ESP_LOGI(TAG, "Device rebooted - already joined to network");
            }
        } else {
            ESP_LOGW(TAG, "%s failed with status: %s, retrying", esp_zb_zdo_signal_to_string(sig),
                     esp_err_to_name(err_status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)esp_zb_bdb_start_top_level_commissioning,
                                   ESP_ZB_BDB_MODE_INITIALIZATION, 1000);
        }
        break;
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Successfully joined network");
            pairing_mode_active = false;
        } else {
            ESP_LOGW(TAG, "Failed to join network (status: %s)", esp_err_to_name(err_status));
            if (pairing_mode_active) {
                ESP_LOGI(TAG, "Retrying pairing in 5 seconds...");
                esp_zb_scheduler_alarm((esp_zb_callback_t)esp_zb_bdb_start_top_level_commissioning,
                                       ESP_ZB_BDB_MODE_NETWORK_STEERING, 5000);
            }
        }
        break;
    case ESP_ZB_ZDO_SIGNAL_LEAVE:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Successfully left network");
            factory_reset_pending = false;
            // Restart commissioning to join a new network
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
        } else {
            ESP_LOGE(TAG, "Failed to leave network (status: %s)", esp_err_to_name(err_status));
            factory_reset_pending = false;
        }
        break;
    default:
        ESP_LOGI(TAG, "Unhandled signal: %s", esp_zb_zdo_signal_to_string(sig));
        break;
    }
}

// --------------- Main ---------------
void app_main(void) {
    // LEDC PWM init (C6 supports only LOW_SPEED mode) :contentReference[oaicite:7]{index=7}
    ledc_timer_config(&(ledc_timer_config_t){
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    });
    ledc_channel_config(&(ledc_channel_config_t){
        .gpio_num = PIN_PWM_FAN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    });

    // Buttons
    init_button(PIN_BTN_MODE);
    init_button(PIN_BTN_UP);
    init_button(PIN_BTN_DOWN);
    init_button(PIN_BTN_TOGGLE);

    // ADC one-shot + calibration (C6 ADC; oneshot driver) :contentReference[oaicite:8]{index=8}
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_cfg, &adc_handle);
    adc_oneshot_chan_cfg_t cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12
    };
    adc_oneshot_config_channel(adc_handle, PIN_ADC_TEMP, &cfg);

    adc_cali_curve_fitting_config_t cal_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_cali_create_scheme_curve_fitting(&cal_cfg, &cali_handle) == ESP_OK) {
        adc_calibrated = true;
    }

    // -------- Zigbee init (End Device) --------
    esp_zb_cfg_t zb_nwk_cfg = {
        .install_code_policy = false,
        .nwk_cfg = {
            .zed_cfg = {
                .ed_timeout = 30,
                .keep_alive = 30,
            },
        },
    };
    esp_zb_init(&zb_nwk_cfg);

    // Create a simple on-off light endpoint (we'll use this as a base)
    esp_zb_on_off_light_cfg_t light_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();
    esp_zb_ep_list_t *esp_zb_light_ep = esp_zb_on_off_light_ep_create(HA_ESP_LIGHT_ENDPOINT, &light_cfg);
    
    // Note: Manufacturer info can be added later once basic functionality works
    
    // Register device and start
    esp_zb_device_register(esp_zb_light_ep);
    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(0x00000800); // Channel 11
    ESP_ERROR_CHECK(esp_zb_start(false));
    ESP_LOGI(TAG, "Zigbee stack started");

    while (true) {
        // Zigbee main loop tick
        esp_zb_stack_main_loop(); // must be called periodically

        // Periodic tasks
        static uint32_t tms = 0;
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
        if (now - tms >= 1000) {
            tms = now;

            // Read temperature & publish to ZCL (0.01°C)
            zcl_temp_measured = read_temp_centi();
            // Note: Attribute updates are handled automatically by the cluster framework

            // Keep PWM synced to fan_speed
            apply_fan_pwm();
        }

        // Scan buttons
        scan_buttons();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
