#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/activity.h>
#include <zmk/ble.h>
#include <zmk/usb.h>
#include <zmk/battery.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) || !IS_ENABLED(CONFIG_ZMK_SPLIT)
#include <zmk/keymap.h>
#endif

#include <zmk/events/activity_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/events/battery_state_changed.h>

#include <status_led/status_led.h>
#include <status_led/status_led_config.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) || !IS_ENABLED(CONFIG_ZMK_SPLIT)
static struct status_led_rgb get_layer_color(uint8_t layer) {
    uint32_t hex = CONFIG_STATUS_LED_LAYER_DEFAULT_COLOR_HEX;
    if (layer == 1) hex = CONFIG_STATUS_LED_LAYER_1_COLOR_HEX;
    else if (layer == 2) hex = CONFIG_STATUS_LED_LAYER_2_COLOR_HEX;
    else if (layer == 3) hex = CONFIG_STATUS_LED_LAYER_3_COLOR_HEX;
    
    return (struct status_led_rgb){
        .r = (hex >> 16) & 0xFF,
        .g = (hex >> 8) & 0xFF,
        .b = hex & 0xFF
    };
}
#endif


static int status_led_event_listener(const zmk_event_t *eh) {
    // 1. Activity State (Sleep/Wake)
    const struct zmk_activity_state_changed *activity_ev = as_zmk_activity_state_changed(eh);
    if (activity_ev != NULL) {
        if (activity_ev->state == ZMK_ACTIVITY_SLEEP) {
            LOG_INF("Entering sleep, powering off status LEDs");
            status_led_sleep();
        } else {
            LOG_INF("Waking up status LEDs");
            status_led_wake();
        }
        return 0;
    }

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) || !IS_ENABLED(CONFIG_ZMK_SPLIT)
    // 2. BLE Profile Changed
    const struct zmk_ble_active_profile_changed *ble_ev = as_zmk_ble_active_profile_changed(eh);
    if (ble_ev != NULL) {
        struct status_led_rgb color;
        switch (ble_ev->index + 1) {
            case 1: color = STATUS_LED_COLOR_PROFILE_1; break;
            case 2: color = STATUS_LED_COLOR_PROFILE_2; break;
            case 3: color = STATUS_LED_COLOR_PROFILE_3; break;
            case 4: color = STATUS_LED_COLOR_PROFILE_4; break;
            case 5: color = STATUS_LED_COLOR_PROFILE_5; break;
            default: color = STATUS_LED_COLOR_PROFILE_1; break;
        }
        LOG_INF("BLE profile changed to %d, showing profile color", ble_ev->index + 1);
        struct status_led_animation anim = {
            .type = STATUS_LED_ANIM_SOLID,
            .color = color,
            .duration_ms = STATUS_LED_DURATION_PROFILE_MS,
            .priority = STATUS_LED_PRIO_BLE_PROFILE,
            .led_mask = STATUS_LED_MASK_LEFT_CENTER | STATUS_LED_MASK_RIGHT_CENTER
        };
        status_led_play(&anim);
        return 0;
    }

    // 3. Layer Changed
    const struct zmk_layer_state_changed *layer_ev = as_zmk_layer_state_changed(eh);
    if (layer_ev != NULL) {
        if (layer_ev->state) { // Only when a layer is activated
            LOG_INF("Layer changed to %d, showing layer color", layer_ev->layer);
            struct status_led_animation anim = {
                .type = STATUS_LED_ANIM_SOLID,
                .color = get_layer_color(layer_ev->layer),
                .duration_ms = STATUS_LED_DURATION_LAYER_MS,
                .priority = STATUS_LED_PRIO_LAYER,
                .led_mask = STATUS_LED_MASK_LEFT_OUTER | STATUS_LED_MASK_RIGHT_OUTER
            };
            status_led_play(&anim);
        }
        return 0;
    }
#endif


    // 4. Split Connection Changed
    const struct zmk_split_peripheral_status_changed *split_ev = as_zmk_split_peripheral_status_changed(eh);
    if (split_ev != NULL) {
        if (split_ev->connected) {
            LOG_INF("Split peripheral connected, showing split conn animation");
            struct status_led_animation anim = {
                .type = STATUS_LED_ANIM_SOLID,
                .color = STATUS_LED_COLOR_GREEN,
                .duration_ms = STATUS_LED_DURATION_SPLIT_CONN_MS,
                .priority = STATUS_LED_PRIO_SPLIT_CONN,
                .led_mask = STATUS_LED_MASK_LEFT_CENTER | STATUS_LED_MASK_RIGHT_CENTER
            };
            status_led_play(&anim);
        }
        return 0;
    }

    // 5. USB Connection / Charging Changed
    const struct zmk_usb_conn_state_changed *usb_ev = as_zmk_usb_conn_state_changed(eh);
    if (usb_ev != NULL) {
        bool is_powered = (usb_ev->conn_state != ZMK_USB_CONN_NONE);
        if (is_powered) {
            uint8_t battery_level = zmk_battery_state_of_charge();
            LOG_INF("USB connected (charging state). Level: %d", battery_level);
            struct status_led_animation anim = {
                .color = STATUS_LED_COLOR_GREEN,
                .priority = STATUS_LED_PRIO_CHARGING,
                .led_mask = STATUS_LED_MASK_LEFT_INNER | STATUS_LED_MASK_RIGHT_INNER
            };
            if (battery_level >= 100) {
                anim.type = STATUS_LED_ANIM_SOLID;
                anim.duration_ms = STATUS_LED_DURATION_CHARGING_MS;
            } else {
                anim.type = STATUS_LED_ANIM_PULSE;
                anim.duration_ms = STATUS_LED_DURATION_CHARGING_MS;
            }
            status_led_play(&anim);
        }
        return 0;
    }

    // 6. Battery Level Changed
    const struct zmk_battery_state_changed *battery_ev = as_zmk_battery_state_changed(eh);
    if (battery_ev != NULL) {
        uint8_t pct = battery_ev->state_of_charge;
        LOG_INF("Battery percentage changed to %d%%", pct);
        struct status_led_rgb color;
        if (pct > 75) color = STATUS_LED_COLOR_BATTERY_HIGH;
        else if (pct >= 30) color = STATUS_LED_COLOR_BATTERY_MED;
        else color = STATUS_LED_COLOR_BATTERY_LOW;

        // If battery is critical (< 30%), show critical indication.
        // Otherwise, only indicate battery level on explicit changes or check.
        if (pct < 30) {
            struct status_led_animation anim = {
                .type = STATUS_LED_ANIM_FLASH,
                .color = color,
                .duration_ms = STATUS_LED_DURATION_BATTERY_MS,
                .priority = STATUS_LED_PRIO_BATTERY_CRIT,
                .led_mask = STATUS_LED_MASK_LEFT_INNER | STATUS_LED_MASK_RIGHT_INNER
            };
            status_led_play(&anim);
        }
        return 0;
    }

    return 0;
}

ZMK_LISTENER(status_led_module, status_led_event_listener);
ZMK_SUBSCRIPTION(status_led_module, zmk_activity_state_changed);
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) || !IS_ENABLED(CONFIG_ZMK_SPLIT)
ZMK_SUBSCRIPTION(status_led_module, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(status_led_module, zmk_layer_state_changed);
#endif

ZMK_SUBSCRIPTION(status_led_module, zmk_split_peripheral_status_changed);
ZMK_SUBSCRIPTION(status_led_module, zmk_usb_conn_state_changed);
ZMK_SUBSCRIPTION(status_led_module, zmk_battery_state_changed);

static int status_led_module_init(void) {
    int err = status_led_init();
    if (err) {
        LOG_ERR("Failed to initialize status LED framework: %d", err);
        return err;
    }

    LOG_INF("ZMK Status LED Framework initialized. Playing BOOT animation.");

    // Trigger boot animation
    struct status_led_animation boot_anim = {
        .type = STATUS_LED_ANIM_FADE,
        .color = STATUS_LED_COLOR_GREEN,
        .duration_ms = STATUS_LED_DURATION_BOOT_MS,
        .priority = STATUS_LED_PRIO_BOOT,
        .led_mask = STATUS_LED_MASK_LEFT_CENTER | STATUS_LED_MASK_RIGHT_CENTER
    };
    status_led_play(&boot_anim);

    return 0;
}

SYS_INIT(status_led_module_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
