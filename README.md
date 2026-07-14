# ZMK Status LED Framework

An event-driven, battery-efficient, and highly customizable status indicator LED framework for [ZMK Firmware](https://zmk.dev/).

---

## 1. Overview
The **ZMK Status LED Framework** manages keyboard status indicators using addressable RGB LEDs (e.g., WS2812, SK6812). Key features:
*   **Contextual Display:** LEDs remain OFF during regular use and wake only to display temporary animation feedback for key events.
*   **Virtual LED abstraction:** Separate logical layouts from physical indices.
*   **Priority-based overrides:** High-priority animations (like charging or errors) interrupt lower-priority animations.
*   **Battery efficient:** Zero polling; strictly event-driven.

---

## 2. Virtual LED Model
The framework exposes six logical LEDs:
*   `LEFT_OUTER`
*   `LEFT_CENTER`
*   `LEFT_INNER`
*   `RIGHT_INNER`
*   `RIGHT_CENTER`
*   `RIGHT_OUTER`

The physical index mapping is resolved in the Hardware Mapping Layer (`hw_mapping.c`), completely isolating physical index offsets from application logic.

---

## 3. Supported Events & Animations

| Priority | Event | Trigger | Default Behavior | Duration |
|----------|-------|---------|------------------|----------|
| 0 (Critical)| Error | Firmware errors / reset loops | Red Flash (3 repetitions) | 1.5 seconds |
| 1 (High)  | Charging | USB plugged in | Green Pulse (Charging) / Solid Green (Full) | 3 seconds |
| 2 (High)  | Battery Critical | Charge drops < 30% | Red Flash | 2 seconds |
| 3 (Medium)| Boot | Powering on | Left & Right Center Green Fade Out | 1 second |
| 4 (Medium)| BLE Profile Changed | BLE profile switched | Blue/Cyan/Purple/White/Orange | 2 seconds |
| 5 (Medium)| Split Connected | Inter-half BLE sync | Left & Right Center Solid Green | 1 second |
| 6 (Low)   | Layer Changed | Activating new keymap layer | Configurable per-layer RGB hex | 1 second |
| 7 (Lowest)| Battery Check | Dedicated binding / manual query | Green (>75%), Yellow (30-75%), Red (<30%) | 2 seconds |

---

## 4. Configuration Guide

Add the module to your `config/west.yml`:

```yaml
manifest:
  projects:
    - name: zmk-status-led-framework
      url: https://github.com/WillACosta/zmk-status-led-framework
      revision: main
```

Enable the framework in your shield or keyboard `.conf` files:
```ini
# Enable the framework
CONFIG_ZMK_STATUS_LED=y

# Configure physical mapping for the Left half (e.g., eyelash_corne_left.conf)
CONFIG_STATUS_LED_SIDE_LEFT=y
CONFIG_STATUS_LED_LEFT_OUTER_IDX=0
CONFIG_STATUS_LED_LEFT_CENTER_IDX=1
CONFIG_STATUS_LED_LEFT_INNER_IDX=2

# Configure physical mapping for the Right half (e.g., eyelash_corne_right.conf)
CONFIG_STATUS_LED_SIDE_RIGHT=y
CONFIG_STATUS_LED_RIGHT_INNER_IDX=3
CONFIG_STATUS_LED_RIGHT_CENTER_IDX=4
CONFIG_STATUS_LED_RIGHT_OUTER_IDX=5
```

---

## 5. Architecture & Customization Guide
Refer to the engineering docs located in the `/docs` directory for complete details on the architecture proposal and design decisions.
