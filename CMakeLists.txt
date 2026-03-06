cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(spi-to-can-transceiver C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

# force no optimizations
set(PICO_DEOPTIMIZED_DEBUG 1)

add_executable(mcp2518fd
    src/mcp2518fd.c
)

target_link_libraries(mcp2518fd
    pico_stdlib
    hardware_gpio
    hardware_timer
    hardware_resets
    hardware_clocks
    hardware_spi
)

pico_enable_stdio_usb(mcp2518fd 1)
pico_enable_stdio_uart(mcp2518fd 0)

pico_add_extra_outputs(mcp2518fd)

target_compile_definitions(mcp2518fd PRIVATE
    PICO_RP2350=1
    PICO_DEBUG=1
)

# be explicit about using rp2350
target_include_directories(mcp2518fd PRIVATE
    ${PICO_SDK_PATH}/src/rp2350/hardware_regs/include
    ${PICO_SDK_PATH}/src/rp2350/hardware_structs/include
)

# remove optimization for debugging
target_compile_options(mcp2518fd PRIVATE
    -g3
    -O0
    -fno-inline
)