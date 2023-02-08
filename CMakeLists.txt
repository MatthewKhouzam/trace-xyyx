cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()
add_executable(traced-xyyx)

if (TARGET tinyusb_device)
    target_sources(traced-xyyx PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/xyyx.c
        ${CMAKE_CURRENT_LIST_DIR}/ctf_pi.c
            )



    # enable usb output, disable uart output
    pico_enable_stdio_usb(traced-xyyx 1)
    pico_enable_stdio_uart(traced-xyyx 0)
    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(traced-xyyx)

    # pull in common dependencies
    target_link_libraries(traced-xyyx PUBLIC pico_types pico_stdlib pico_multicore hardware_rtc hardware_timer tinyusb_device pico_time)

#    target_link_libraries(traced-xyyx PUBLIC pico_types pico_stdlib tinyusb_host  pico_multicore hardware_rtc hardware_uart hardware_timer)
elseif(PICO_ON_DEVICE)
    message(WARNING "not building traced-xyyx because TinyUSB submodule is not initialized in the SDK")
endif()
