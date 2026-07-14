# Architecture Proposal: ZMK Status LED Framework

## 1. Design Overview
The **ZMK Status LED Framework** is an event-driven, hardware-abstracted, and energy-efficient module designed to display system statuses via LEDs without running continuous underglow or decorative animations.

---

## 2. Virtual LED Model & Hardware Mapping
We expose 6 virtual LEDs:
```c
enum status_led_id {
    STATUS_LED_LEFT_OUTER,
    STATUS_LED_LEFT_CENTER,
    STATUS_LED_LEFT_INNER,
    STATUS_LED_RIGHT_INNER,
    STATUS_LED_RIGHT_CENTER,
    STATUS_LED_RIGHT_OUTER,
    STATUS_LED_COUNT
};
```
Each keyboard configuration maps these logical IDs to physical LED strip indices. In split keyboards, we configure which side is active:
*   **Left Half:** Controls `STATUS_LED_LEFT_OUTER`, `STATUS_LED_LEFT_CENTER`, `STATUS_LED_LEFT_INNER`.
*   **Right Half:** Controls `STATUS_LED_RIGHT_INNER`, `STATUS_LED_RIGHT_CENTER`, `STATUS_LED_RIGHT_OUTER`.

The mapping is defined in the configuration layer:
```c
struct status_led_map {
    enum status_led_id logical_id;
    uint32_t physical_index;
};
```

---

## 3. Public API
```c
#pragma once

#include <stdint.h>

enum status_led_event {
    STATUS_LED_EV_BOOT,
    STATUS_LED_EV_BLE_PROFILE,
    STATUS_LED_EV_LAYER,
    STATUS_LED_EV_BATTERY,
    STATUS_LED_EV_CHARGING,
    STATUS_LED_EV_SPLIT_CONN,
    STATUS_LED_EV_ERROR
};

int status_led_show(enum status_led_event ev, uint32_t param);
int status_led_clear(void);
int status_led_sleep(void);
```

---

## 4. Animation Renderer
The renderer runs a low-priority Zephyr work queue item or timer (`k_timer`) to update the LED states during an active animation.
*   **Solid:** Turn LEDs on to a static color.
*   **Flash:** Toggle LEDs on/off periodically.
*   **Pulse:** Gradually increase and decrease brightness using a sine wave.
*   **Fade:** Gradually fade out brightness to 0 over a duration.

Animations are configured with:
*   **Color:** HSB (Hue, Saturation, Brightness) or RGB.
*   **Duration:** Timeout in milliseconds.
*   **Priority:** Critical > High > Medium > Low.

---

## 5. Event Subscriptions & Integration
*   The framework registers as a ZMK event listener.
*   It subscribes to standard events and triggers corresponding status indications.
*   No changes are required to ZMK core, keeping the module 100% upstream-friendly.
