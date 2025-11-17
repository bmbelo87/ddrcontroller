cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_FETCH_FROM_GIT ON)
include("/home/bruno/pico-sdk/external/pico_sdk_import.cmake")

project(ddrcontroller C CXX ASM)

pico_sdk_init()

add_executable(ddrcontroller
    main.c
    descriptors/usb_descriptors.c
)

target_sources(ddrcontroller PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/descriptors/usb_descriptors.c
        )

target_include_directories(ddrcontroller PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(ddrcontroller
    pico_stdlib
    hardware_gpio
    hardware_adc
    pico_multicore
    tinyusb_board
    tinyusb_device
)

    pico_add_extra_outputs(ddrcontroller)
    pico_enable_stdio_usb(ddrcontroller 0)
    pico_enable_stdio_uart(ddrcontroller 0)           

    message(STATUS "Configured DDRController. Make sure PICO_SDK_PATH is set correctly.")
