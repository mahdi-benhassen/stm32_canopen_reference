cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32f767_canopen)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# ------------------------------------------------------------------
# CANopen reference integration (pinned CanOpenSTM32 submodule).
# The HAL/CMSIS vendor code stays inside the generated STM32_Drivers
# library; only stack, driver-binding, OD, middleware, and application
# sources are added to the executable here.
# ------------------------------------------------------------------
set(CANOPEN_STM32_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/CanOpenSTM32)
set(CANOPEN_NODE_DIR ${CANOPEN_STM32_DIR}/CANopenNode)

file(GLOB CANOPEN_NODE_SOURCES CONFIGURE_DEPENDS
    ${CANOPEN_NODE_DIR}/301/*.c
    ${CANOPEN_NODE_DIR}/303/*.c
    ${CANOPEN_NODE_DIR}/304/*.c
    ${CANOPEN_NODE_DIR}/305/*.c
    ${CANOPEN_NODE_DIR}/309/*.c
    ${CANOPEN_NODE_DIR}/storage/CO_storage.c)

option(CANOPEN_REFERENCE_PRODUCTION_PROFILE "Enable production optimization and compiler hardening" ON)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    App/Src/CO_app_STM32_reference.c
    App/Src/canopen_reference_board.c
    App/Src/canopen_reference_can_recovery.c
    App/Src/canopen_reference_cia302.c
    App/Src/canopen_reference_diagnostics.c
    App/Src/canopen_reference_gateway.c
    App/Src/canopen_reference_hw.c
    App/Src/canopen_reference_lss.c
    App/Src/canopen_reference_port_fixup.c
    App/Src/canopen_reference_storage.c
    App/Src/canopen_reference_timing.c
    App/Src/canopen_reference_watchdog.c
    App/Src/cia401_reference.c
    App/Src/cia402_reference.c
    App/Src/cia418_reference.c
    App/Src/inventus_battery_data.c
    middleware/canopen/core/can_acceptance_filter.c
    middleware/canopen/core/canopen_core.c
    middleware/canopen/core/cia302_nmt_master.c
    Generated/OD.c
    ${CANOPEN_STM32_DIR}/CANopenNode_STM32/CO_driver_STM32.c
    ${CANOPEN_NODE_DIR}/CANopen.c
    ${CANOPEN_NODE_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    App/Inc
    Generated
    middleware/canopen/core
    ${CANOPEN_STM32_DIR}/CANopenNode_STM32
    ${CANOPEN_NODE_DIR}
    ${CANOPEN_NODE_DIR}/301
    ${CANOPEN_NODE_DIR}/303
    ${CANOPEN_NODE_DIR}/304
    ${CANOPEN_NODE_DIR}/305
    ${CANOPEN_NODE_DIR}/309
    ${CANOPEN_NODE_DIR}/storage
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    # Note: ART_ACCELERATOR_ENABLE / USE_HAL_*_REGISTER_CALLBACKS are owned by
    # the generated Core/Inc/stm32f7xx_hal_conf.h on this branch; redefining
    # them on the command line would fail -Werror.
    CO_DRIVER_CUSTOM
    CO_USE_GLOBALS
    __int64_t_defined=1
)

if(CANOPEN_REFERENCE_PRODUCTION_PROFILE)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Werror
        -Wformat=2 -Wconversion -Wshadow -Wundef -Werror=conversion)
    # Dependency-class sources (pinned stack, driver binding, generated OD)
    # keep their warnings visible but must not defeat fail-on-warning for
    # project-owned code. HAL/CMSIS stay in the generated STM32_Drivers
    # library and are therefore untouched by this list.
    set_source_files_properties(
        ${CANOPEN_NODE_SOURCES}
        "${CANOPEN_NODE_DIR}/CANopen.c"
        "${CANOPEN_STM32_DIR}/CANopenNode_STM32/CO_driver_STM32.c"
        Generated/OD.c
        Core/Src/syscalls.c
        PROPERTIES COMPILE_OPTIONS "-Wno-error=conversion;-Wno-error=sign-conversion;-Wno-error=format;-Wno-error=unused-parameter;-Wno-error=unused-variable;-Wno-error=undef")
endif()

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
