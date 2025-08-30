#ifndef ZIGBEE_H
#define ZIGBEE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "esp_zigbee_core.h"
#include "esp_zigbee_cluster.h"
#include "esp_zigbee_endpoint.h"

// Zigbee state
extern bool pairing_mode_active;
extern bool factory_reset_pending;
extern bool zb_joined;

// Endpoint used by our device
#define HA_ESP_LIGHT_ENDPOINT 10

// Add vendor information constants at the top after the includes
#define MANUFACTURER_NAME               "\x0C""SiloCityLabs"
#define MODEL_IDENTIFIER                "\x0F""airtap-4btn-rev2"
#define SW_BUILD_ID                     "\x08""1.0.0"

// Function prototypes
void zigbee_init(void);
void zigbee_start_pairing(void);
void zigbee_cancel_pairing(void);
void zigbee_factory_reset(void);
void zigbee_task(void *pvParameters);

// Zigbee signal handler (called from main)
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct);

#endif // ZIGBEE_H
