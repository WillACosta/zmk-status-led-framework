#pragma once

#include <status_led/status_led.h>

// Profile colors (RGB)
#define STATUS_LED_COLOR_PROFILE_1 (struct status_led_rgb){0, 0, 255}    // Blue
#define STATUS_LED_COLOR_PROFILE_2 (struct status_led_rgb){0, 255, 255}  // Cyan
#define STATUS_LED_COLOR_PROFILE_3 (struct status_led_rgb){255, 0, 255}  // Purple
#define STATUS_LED_COLOR_PROFILE_4 (struct status_led_rgb){255, 255, 255}// White
#define STATUS_LED_COLOR_PROFILE_5 (struct status_led_rgb){255, 127, 0}  // Orange

// Battery level colors (RGB)
#define STATUS_LED_COLOR_BATTERY_HIGH (struct status_led_rgb){0, 255, 0}   // Green
#define STATUS_LED_COLOR_BATTERY_MED  (struct status_led_rgb){255, 255, 0} // Yellow
#define STATUS_LED_COLOR_BATTERY_LOW  (struct status_led_rgb){255, 0, 0}   // Red

// General colors
#define STATUS_LED_COLOR_GREEN (struct status_led_rgb){0, 255, 0}
#define STATUS_LED_COLOR_RED   (struct status_led_rgb){255, 0, 0}
#define STATUS_LED_COLOR_OFF   (struct status_led_rgb){0, 0, 0}

// Durations
#define STATUS_LED_DURATION_BOOT_MS        1000
#define STATUS_LED_DURATION_PROFILE_MS     2000
#define STATUS_LED_DURATION_LAYER_MS       1000
#define STATUS_LED_DURATION_BATTERY_MS      2000
#define STATUS_LED_DURATION_SPLIT_CONN_MS  1000
#define STATUS_LED_DURATION_ERROR_MS       1500 // 3 flashes of 250ms on/off
#define STATUS_LED_DURATION_CHARGING_MS    3000

// LED Masks
#define STATUS_LED_MASK_LEFT_OUTER   (1 << STATUS_LED_LEFT_OUTER)
#define STATUS_LED_MASK_LEFT_CENTER  (1 << STATUS_LED_LEFT_CENTER)
#define STATUS_LED_MASK_LEFT_INNER   (1 << STATUS_LED_LEFT_INNER)
#define STATUS_LED_MASK_RIGHT_INNER  (1 << STATUS_LED_RIGHT_INNER)
#define STATUS_LED_MASK_RIGHT_CENTER (1 << STATUS_LED_RIGHT_CENTER)
#define STATUS_LED_MASK_RIGHT_OUTER  (1 << STATUS_LED_RIGHT_OUTER)

#define STATUS_LED_MASK_LEFT_ALL  (STATUS_LED_MASK_LEFT_OUTER | STATUS_LED_MASK_LEFT_CENTER | STATUS_LED_MASK_LEFT_INNER)
#define STATUS_LED_MASK_RIGHT_ALL (STATUS_LED_MASK_RIGHT_INNER | STATUS_LED_MASK_RIGHT_CENTER | STATUS_LED_MASK_RIGHT_OUTER)
#define STATUS_LED_MASK_ALL       (STATUS_LED_MASK_LEFT_ALL | STATUS_LED_MASK_RIGHT_ALL)
