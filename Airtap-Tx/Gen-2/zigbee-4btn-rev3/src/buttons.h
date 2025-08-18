#ifndef BUTTONS_H
#define BUTTONS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

// Pin definitions
#define PIN_BTN_MODE    18
#define PIN_BTN_UP      17
#define PIN_BTN_DOWN    19
#define PIN_BTN_TOGGLE  20

// Button states
typedef enum {
    BUTTON_UP = 0,
    BUTTON_DOWN = 1
} button_state_t;

// Button events
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_UP_PRESS,
    BUTTON_EVENT_DOWN_PRESS,
    BUTTON_EVENT_TOGGLE_PRESS,
    BUTTON_EVENT_TOGGLE_LONG_PRESS,
    BUTTON_EVENT_MODE_PRESS
} button_event_t;

// Function prototypes
void buttons_init(void);
button_event_t buttons_scan(void);
void buttons_handle_event(button_event_t event);

#endif // BUTTONS_H
