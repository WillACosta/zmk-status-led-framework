#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <status_led/status_led.h>

#define DT_DRV_COMPAT zmk_behavior_status_led_battery

static int behavior_status_led_battery_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                                             struct zmk_behavior_binding_event event) {
    extern void status_led_trigger_battery_check(void);
    status_led_trigger_battery_check();
    return 0;
}

static int behavior_status_led_battery_keymap_binding_released(struct zmk_behavior_binding *binding,
                                                              struct zmk_behavior_binding_event event) {
    return 0;
}

static const struct behavior_driver_api behavior_status_led_battery_api = {
    .binding_pressed = behavior_status_led_battery_keymap_binding_pressed,
    .binding_released = behavior_status_led_battery_keymap_binding_released,
};

static int behavior_status_led_battery_init(const struct device *dev) {
    return 0;
}

BEHAVIOR_DT_INST_DEFINE(0, behavior_status_led_battery_init, NULL, NULL, NULL,
                        APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_status_led_battery_api);
