#pragma once

#include <stdint.h>
#include <stdbool.h>

enum status_led_id {
    STATUS_LED_LEFT_OUTER = 0,
    STATUS_LED_LEFT_CENTER = 1,
    STATUS_LED_LEFT_INNER = 2,
    STATUS_LED_RIGHT_INNER = 3,
    STATUS_LED_RIGHT_CENTER = 4,
    STATUS_LED_RIGHT_OUTER = 5,
    STATUS_LED_COUNT = 6
};

struct status_led_rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

enum status_led_anim_type {
    STATUS_LED_ANIM_SOLID,
    STATUS_LED_ANIM_FLASH,
    STATUS_LED_ANIM_PULSE,
    STATUS_LED_ANIM_FADE
};

enum status_led_priority {
    STATUS_LED_PRIO_BOOT = 0,
    STATUS_LED_PRIO_ERROR,
    STATUS_LED_PRIO_CHARGING,
    STATUS_LED_PRIO_BATTERY_CRIT,
    STATUS_LED_PRIO_BLE_PROFILE,
    STATUS_LED_PRIO_SPLIT_CONN,
    STATUS_LED_PRIO_LAYER,
    STATUS_LED_PRIO_BATTERY_CHECK,
    STATUS_LED_PRIO_COUNT
};

struct status_led_animation {
    enum status_led_anim_type type;
    struct status_led_rgb color;
    uint32_t duration_ms;
    enum status_led_priority priority;
    uint8_t led_mask; // Bitmask of logical LEDs (1 << status_led_id)
};

/**
 * @brief Initialize the status LED framework.
 */
int status_led_init(void);

/**
 * @brief Play an animation on specified status LEDs.
 * 
 * If a higher or equal priority animation is already playing,
 * the new animation might be queued or override it based on priority.
 */
int status_led_play(const struct status_led_animation *anim);

/**
 * @brief Clear all status LEDs (turn off).
 */
int status_led_clear(void);

/**
 * @brief Put status LEDs into low-power sleep mode.
 */
int status_led_sleep(void);

/**
 * @brief Wake status LEDs from sleep mode.
 */
int status_led_wake(void);
