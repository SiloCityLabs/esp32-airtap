#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "driver/i2c.h"
#include "esp_log.h"
#include <string.h>

// Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

// Function prototypes
void oled_init(void);
void oled_clear(void);
void oled_draw_text(int x, int y, const char *text);
void oled_update_display(float temp_c, int fan_speed, bool pairing_active, bool factory_reset_pending, bool zb_joined, uint32_t uptime_seconds);

#endif // OLED_DISPLAY_H
