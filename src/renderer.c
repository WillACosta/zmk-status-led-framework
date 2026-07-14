#include <zephyr/kernel.h>
#include <status_led/status_led.h>
#include <status_led/status_led_config.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

extern int status_led_hw_init(void);
extern int status_led_hw_set_color(enum status_led_id logical_id, struct status_led_rgb color);
extern int status_led_hw_update(void);

// Renderer status
static struct status_led_animation current_anim = {
    .priority = STATUS_LED_PRIO_COUNT // Use COUNT as "none" priority (lowest value represents highest priority)
};
static uint32_t anim_elapsed_ms = 0;
static bool is_sleeping = false;

static struct k_work_delayable anim_work;

static void anim_work_handler(struct k_work *work) {
    if (is_sleeping || current_anim.priority >= STATUS_LED_PRIO_COUNT) {
        return;
    }

    anim_elapsed_ms += 50;

    if (anim_elapsed_ms >= current_anim.duration_ms) {
        // Animation finished
        current_anim.priority = STATUS_LED_PRIO_COUNT;
        
        // Turn off LEDs
        for (int i = 0; i < STATUS_LED_COUNT; i++) {
            status_led_hw_set_color(i, STATUS_LED_COLOR_OFF);
        }
        status_led_hw_update();
        return;
    }

    // Calculate current color based on animation type
    struct status_led_rgb active_color = STATUS_LED_COLOR_OFF;

    switch (current_anim.type) {
        case STATUS_LED_ANIM_SOLID:
            active_color = current_anim.color;
            break;

        case STATUS_LED_ANIM_FLASH:
            // Toggle every 250ms
            if ((anim_elapsed_ms / 250) % 2 == 0) {
                active_color = current_anim.color;
            } else {
                active_color = STATUS_LED_COLOR_OFF;
            }
            break;

        case STATUS_LED_ANIM_PULSE: {
            // Triangle wave pulse over 1000ms period
            uint32_t phase = anim_elapsed_ms % 1000;
            float factor = 0.0f;
            if (phase < 500) {
                factor = phase / 500.0f;
            } else {
                factor = (1000 - phase) / 500.0f;
            }
            active_color.r = (uint8_t)(current_anim.color.r * factor);
            active_color.g = (uint8_t)(current_anim.color.g * factor);
            active_color.b = (uint8_t)(current_anim.color.b * factor);
            break;
        }

        case STATUS_LED_ANIM_FADE: {
            // Linear fade out
            float factor = 1.0f - ((float)anim_elapsed_ms / current_anim.duration_ms);
            if (factor < 0.0f) factor = 0.0f;
            active_color.r = (uint8_t)(current_anim.color.r * factor);
            active_color.g = (uint8_t)(current_anim.color.g * factor);
            active_color.b = (uint8_t)(current_anim.color.b * factor);
            break;
        }
    }

    // Apply color to masked LEDs, OFF to others
    for (int i = 0; i < STATUS_LED_COUNT; i++) {
        if (current_anim.led_mask & (1 << i)) {
            status_led_hw_set_color(i, active_color);
        } else {
            status_led_hw_set_color(i, STATUS_LED_COLOR_OFF);
        }
    }

    status_led_hw_update();

    // Reschedule next frame in 50ms
    k_work_reschedule(&anim_work, K_MSEC(50));
}

int status_led_init(void) {
    int err = status_led_hw_init();
    if (err) return err;

    current_anim.priority = STATUS_LED_PRIO_COUNT;
    anim_elapsed_ms = 0;
    is_sleeping = false;

    k_work_init_delayable(&anim_work, anim_work_handler);

    return 0;
}

int status_led_play(const struct status_led_animation *anim) {
    if (is_sleeping) return -ENODEV;

    // Check priority: lower enum value is higher priority.
    // If priority is higher or equal, interrupt the current animation.
    if (anim->priority <= current_anim.priority) {
        current_anim = *anim;
        anim_elapsed_ms = 0;
        
        // Schedule work immediately
        k_work_reschedule(&anim_work, K_NO_WAIT);
        return 0;
    }

    return -EBUSY;
}

int status_led_clear(void) {
    current_anim.priority = STATUS_LED_PRIO_COUNT;
    anim_elapsed_ms = 0;
    k_work_cancel_delayable(&anim_work);

    for (int i = 0; i < STATUS_LED_COUNT; i++) {
        status_led_hw_set_color(i, STATUS_LED_COLOR_OFF);
    }
    return status_led_hw_update();
}

int status_led_sleep(void) {
    is_sleeping = true;
    k_work_cancel_delayable(&anim_work);
    
    for (int i = 0; i < STATUS_LED_COUNT; i++) {
        status_led_hw_set_color(i, STATUS_LED_COLOR_OFF);
    }
    return status_led_hw_update();
}

int status_led_wake(void) {
    is_sleeping = false;
    return 0;
}
