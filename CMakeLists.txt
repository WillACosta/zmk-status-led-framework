cmake_minimum_required(VERSION 3.20.0)

zephyr_include_directories(include)

if(CONFIG_ZMK_STATUS_LED)
    target_sources(app PRIVATE
        src/status_led.c
        src/renderer.c
        src/hw_mapping.c
    )
endif()
