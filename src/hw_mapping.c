#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led_strip.h>
#include <status_led/status_led.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_CHOSEN(zmk_underglow)
#define STRIP_CHOSEN DT_CHOSEN(zmk_underglow)
#define STRIP_NUM_PIXELS DT_PROP(STRIP_CHOSEN, chain_length)
#else
#define STRIP_NUM_PIXELS 0
#endif

#if STRIP_NUM_PIXELS > 0
static const struct device *led_strip = DEVICE_DT_GET(STRIP_CHOSEN);
static struct led_rgb pixels[STRIP_NUM_PIXELS];
#endif

static bool hw_initialized = false;

int status_led_hw_init(void) {
#if STRIP_NUM_PIXELS > 0
    if (!device_is_ready(led_strip)) {
        LOG_ERR("LED strip device is not ready");
        return -ENODEV;
    }
    // Set all pixels to off initially
    for (int i = 0; i < STRIP_NUM_PIXELS; i++) {
        pixels[i] = (struct led_rgb){0, 0, 0};
    }
    led_strip_update_rgb(led_strip, pixels, STRIP_NUM_PIXELS);
#endif
    hw_initialized = true;
    return 0;
}

int status_led_hw_set_color(enum status_led_id logical_id, struct status_led_rgb color) {
    if (!hw_initialized) {
        return -EAGAIN;
    }

#if STRIP_NUM_PIXELS > 0
    int physical_idx = -1;

#if IS_ENABLED(CONFIG_STATUS_LED_SIDE_LEFT)
    if (logical_id == STATUS_LED_LEFT_OUTER) {
        physical_idx = CONFIG_STATUS_LED_LEFT_OUTER_IDX;
    } else if (logical_id == STATUS_LED_LEFT_CENTER) {
        physical_idx = CONFIG_STATUS_LED_LEFT_CENTER_IDX;
    } else if (logical_id == STATUS_LED_LEFT_INNER) {
        physical_idx = CONFIG_STATUS_LED_LEFT_INNER_IDX;
    }
#endif

#if IS_ENABLED(CONFIG_STATUS_LED_SIDE_RIGHT)
    if (logical_id == STATUS_LED_RIGHT_INNER) {
        physical_idx = CONFIG_STATUS_LED_RIGHT_INNER_IDX;
    } else if (logical_id == STATUS_LED_RIGHT_CENTER) {
        physical_idx = CONFIG_STATUS_LED_RIGHT_CENTER_IDX;
    } else if (logical_id == STATUS_LED_RIGHT_OUTER) {
        physical_idx = CONFIG_STATUS_LED_RIGHT_OUTER_IDX;
    }
#endif

    if (physical_idx >= 0 && physical_idx < STRIP_NUM_PIXELS) {
        // Brightness scaling
        uint8_t brightness = CONFIG_STATUS_LED_BRIGHTNESS;
        pixels[physical_idx].r = (color.r * brightness) / 100;
        pixels[physical_idx].g = (color.g * brightness) / 100;
        pixels[physical_idx].b = (color.b * brightness) / 100;
    }
#endif

    return 0;
}

int status_led_hw_update(void) {
    if (!hw_initialized) {
        return -EAGAIN;
    }

#if STRIP_NUM_PIXELS > 0
    return led_strip_update_rgb(led_strip, pixels, STRIP_NUM_PIXELS);
#else
    return 0;
#endif
}
