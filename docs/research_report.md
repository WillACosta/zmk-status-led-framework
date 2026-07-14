# Research Report: ZMK Status LED Framework

## 1. Overview
This research report documents the platform audit, source code review, and background analysis of the target ZMK and Zephyr systems to support the implementation of the **ZMK Status LED Framework**.

---

## 2. Platform Audit: Eyelash Corne
The development target is the low-profile **Eyelash Corne** (split 3x6 layout keyboard).

### RGB Led Strip Hardware
*   **Driver:** `worldsemi,ws2812-spi` (WS2812 addressable RGB controller communicating over SPI3).
*   **Chain Length:** 21 pixels total on each half.
*   **Devicetree Node:** `&led_strip` mapped via `zmk,underglow = &led_strip;` in the chosen node.
*   **Color Mapping:** `LED_COLOR_ID_GREEN`, `LED_COLOR_ID_RED`, `LED_COLOR_ID_BLUE` (GRB mapping).

### Power & Battery Subsystem
*   **Battery Driver:** `zmk,battery-nrf-vddh` (NRF52 VDDH voltage monitoring).
*   **External Power:** `zmk,ext-power-generic` on GPIO 0 pin 13 to shut down the LED strip VCC line during sleep.

---

## 3. ZMK & Zephyr Architecture Review

### RGB Underglow Subsystem (`rgb_underglow.c`)
*   Uses Zephyr's `<zephyr/drivers/led_strip.h>` API: `led_strip_update_rgb(led_strip, pixels, count)`.
*   Directly manages a global array of `struct led_rgb` pixels.
*   Implements an animation timer driving periodic updates.
*   Uses `ext_power_enable()` and `ext_power_disable()` to manage the VCC line of the LED strip dynamically.

### ZMK Event System
The framework will hook into the following core ZMK events:
1.  **Layer State Changed:** `zmk_layer_state_changed` (`zmk/events/layer_state_changed.h`)
2.  **BLE Profile Changed:** `zmk_ble_active_profile_changed` (`zmk/events/ble_active_profile_changed.h`)
3.  **Battery Level Changed:** `zmk_battery_state_changed` (`zmk/events/battery_state_changed.h`)
4.  **USB Connection Changed:** `zmk_usb_conn_state_changed` (`zmk/events/usb_conn_state_changed.h`)
5.  **Split Peripheral Status:** `zmk_split_peripheral_status_changed` (`zmk/events/split_peripheral_status_changed.h`)
6.  **Activity (Sleep/Wake):** `zmk_activity_state_changed` (`zmk/events/activity_state_changed.h`)

### Split Synchronization
*   The **Central** side coordinates global states (layers, profiles, active Bluetooth connection).
*   The **Peripheral** side coordinates local hardware events and reports connection status via `zmk_split_peripheral_status_changed`.
*   To support split operation without embedding keyboard-specific synchronization logic, the status LED framework will run local animation scheduling on whichever side receives the event.
