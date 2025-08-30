#include "zigbee.h"
#include "led_control.h"
#include "temperature.h"
#include "oled_display.h"
#include "esp_timer.h"

static const char *TAG = "ZIGBEE";

// Zigbee state variables
bool pairing_mode_active = false;
bool factory_reset_pending = false;
bool zb_joined = false;
static uint8_t pairing_retry_count = 0;

// Retry callback function
static void zigbee_retry_callback(void* arg) {
    if (pairing_mode_active) {
        ESP_LOGI(TAG, "Retrying network steering now (attempt %d/5)", pairing_retry_count);
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
    }
}

// ZCL state variables
static uint8_t zcl_onoff_attr = 0;
static uint8_t zcl_level_attr = 0;
static int16_t zcl_temp_measured = 0;

// Forward declarations
static void trigger_factory_reset(void);
static void trigger_pairing_mode(void);
static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message);
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message);

// Add vendor information constants at the top after the includes
#define MANUFACTURER_NAME               "\x0C""SiloCityLabs"
#define MODEL_IDENTIFIER                "\x0F""airtap-4btn-rev2"
#define SW_BUILD_ID                     "\x08""1.0.0"

void zigbee_init(void) {
    // Initialize Zigbee platform
    esp_zb_platform_config_t config_zb = {
        .radio_config = {
            .radio_mode = ZB_RADIO_MODE_NATIVE,
        },
        .host_config = {
            .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE,
        },
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&config_zb));
    ESP_LOGI(TAG, "Zigbee platform initialized");
}

void zigbee_start_pairing(void) {
    ESP_LOGI(TAG, "Starting Zigbee pairing mode with extended scanning");
    pairing_mode_active = true;
    pairing_retry_count = 0; // Reset retry counter
    
    // Start network steering with extended parameters
    esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
}

void zigbee_cancel_pairing(void) {
    ESP_LOGI(TAG, "Cancelling pairing mode");
    pairing_mode_active = false;
    pairing_retry_count = 0; // Reset retry counter
}

void zigbee_factory_reset(void) {
    ESP_LOGI(TAG, "Factory reset triggered");
    factory_reset_pending = true;
    esp_zb_bdb_reset_via_local_action();
}

static void trigger_factory_reset(void) {
    zigbee_factory_reset();
}

static void trigger_pairing_mode(void) {
    zigbee_start_pairing();
}

// Zigbee signal handler
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct) {
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;
    
    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
            zb_joined = !esp_zb_bdb_is_factory_new();
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Starting network steering (factory new device)");
                pairing_mode_active = true;
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            } else {
                ESP_LOGI(TAG, "Device rebooted - already joined to network");
            }
        } else {
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
        }
        break;
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                     esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
            zb_joined = true;
            pairing_mode_active = false;
            pairing_retry_count = 0; // Reset retry counter on success
        } else {
            ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
            zb_joined = false;
            
            // Implement retry logic with proper timing
            if (pairing_mode_active && pairing_retry_count < 5) {
                pairing_retry_count++;
                ESP_LOGI(TAG, "Will retry network steering in 3 seconds (attempt %d/5)", pairing_retry_count);
                
                // Schedule retry after delay
                esp_timer_handle_t retry_timer;
                esp_timer_create_args_t timer_args = {
                    .callback = zigbee_retry_callback,
                    .name = "zigbee_retry"
                };
                esp_timer_create(&timer_args, &retry_timer);
                esp_timer_start_once(retry_timer, 3000000); // 3 seconds in microseconds
            } else if (pairing_retry_count >= 5) {
                ESP_LOGI(TAG, "Max retries reached, stopping pairing mode");
                pairing_mode_active = false;
                pairing_retry_count = 0;
            }
        }
        break;
    case ESP_ZB_ZDO_SIGNAL_LEAVE:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Device left network");
            zb_joined = false;
            factory_reset_pending = false;
        }
        break;
    default:
        ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                 esp_err_to_name(err_status));
        break;
    }
}

// Zigbee attribute handler
static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message) {
    esp_err_t ret = ESP_OK;
    bool light_state = 0;

    if (!message) {
        ESP_LOGE(TAG, "Empty message");
        return ESP_FAIL;
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster,
             message->attribute.id, message->attribute.data.size);
    
    if (message->info.dst_endpoint == HA_ESP_LIGHT_ENDPOINT) {
        // Handle standard on/off for fan control
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID && message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {
                bool state = message->attribute.data.value ? *(bool *)message->attribute.data.value : false;
                ESP_LOGI(TAG, "Fan state set to %s", state ? "ON" : "OFF");
                zcl_onoff_attr = state ? 1 : 0;
                
                if (state) {
                    led_set(true);
                    fan_set_speed(10); // Turn fan on to max speed
                } else {
                    led_set(false);
                    fan_set_speed(0); // Turn fan off
                }
            }
        }
        // Handle level control for fan speed (0-255 maps to 0-10)
        else if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID && message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_U8) {
                uint8_t level = message->attribute.data.value ? *(uint8_t *)message->attribute.data.value : 0;
                ESP_LOGI(TAG, "Level set to %d", level);
                zcl_level_attr = level;
                
                // Map level (0-255) to fan speed (0-10)
                uint8_t fan_speed = (level * 10) / 255;
                fan_set_speed(fan_speed);
                
                // Update LED state based on level
                led_set(level > 0);
            }
        }
    }
    return ret;
}

// Zigbee action handler
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message) {
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        ret = zb_attribute_handler((esp_zb_zcl_set_attr_value_message_t *)message);
        break;
    default:
        ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

void zigbee_task(void *pvParameters) {
    /* initialize Zigbee stack with custom configuration for longer scanning */
    esp_zb_cfg_t zb_nwk_cfg = {
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED,
        .install_code_policy = false,
        .nwk_cfg.zed_cfg = {
            .ed_timeout = ESP_ZB_ED_AGING_TIMEOUT_64MIN,
            .keep_alive = 3000,
        },
    };
    esp_zb_init(&zb_nwk_cfg);
    
    // Create a custom endpoint with proper vendor information
    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
    
    // Create basic cluster with vendor information
    esp_zb_color_dimmable_light_cfg_t light_cfg = ESP_ZB_DEFAULT_COLOR_DIMMABLE_LIGHT_CONFIG();
    esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(&(light_cfg.basic_cfg));
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, MANUFACTURER_NAME));
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, MODEL_IDENTIFIER));
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_SW_BUILD_ID, SW_BUILD_ID));
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_basic_cluster(cluster_list, basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    
    // Add identify cluster
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_identify_cluster(cluster_list, esp_zb_identify_cluster_create(&(light_cfg.identify_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    
    // Add on/off cluster
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_on_off_cluster(cluster_list, esp_zb_on_off_cluster_create(&(light_cfg.on_off_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    
    // Add level control cluster for fan speed control
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_level_cluster(cluster_list, esp_zb_level_cluster_create(&(light_cfg.level_cfg)), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    
    // Create endpoint with custom clusters
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();
    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = HA_ESP_LIGHT_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_COLOR_DIMMABLE_LIGHT_DEVICE_ID,
        .app_device_version = 0
    };
    esp_zb_ep_list_add_ep(ep_list, cluster_list, endpoint_config);

    // Set Basic cluster Manufacturer/Model so Z2M shows proper info
    // We'll set these attributes after device registration
    ESP_LOGI(TAG, "Device created with manufacturer: SiloCityLabs, model: airtap-4btn-rev2");
    
    // Register device and start
    esp_zb_device_register(ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK); // Scan all channels
    ESP_ERROR_CHECK(esp_zb_start(false));
    
    // Main Zigbee loop
    while (true) {
        esp_zb_main_loop_iteration();
        vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to prevent watchdog issues
    }
}
