# Copyright 2023 Manuel Jedinger
# https://github.com/manujedi
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.25)

set(CMAKE_BUILD_TYPE Debug)

# specify cross compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(firmware ASM C)
set(CMAKE_C_STANDARD 11)

set(MCPU cortex-m0)

add_compile_options(-mcpu=${MCPU} -Wall -Werror -fno-builtin -MMD)
add_compile_options(-DPRINTF_INCLUDE_CONFIG_H)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Size")
    add_compile_options(-Os)
    MESSAGE("Min Size build")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(-Og -g)
    add_link_options(-g)
    MESSAGE("Debug build")
else ()
    add_compile_options(-O2)
    MESSAGE("Other build")
endif ()

include_directories(
        ${CMAKE_SOURCE_DIR}/
        ${CMAKE_SOURCE_DIR}/bsp/.../gpio.h
        ${CMAKE_SOURCE_DIR}/external/CMSIS_5/CMSIS/Core/Include
        ${CMAKE_SOURCE_DIR}/external/CMSIS_5/Device/ARM/ARMCM0/Include
        ${CMAKE_SOURCE_DIR}/driver/
        ${CMAKE_SOURCE_DIR}/gui/
)

file(GLOB SOURCES
        ${CMAKE_SOURCE_DIR}/*.S
        ${CMAKE_SOURCE_DIR}/*.c
        ${CMAKE_SOURCE_DIR}/*.h
        ${CMAKE_SOURCE_DIR}/driver/*.c
        ${CMAKE_SOURCE_DIR}/driver/*.h
        ${CMAKE_SOURCE_DIR}/gui/*.c
        ${CMAKE_SOURCE_DIR}/gui/*.h
        ${CMAKE_SOURCE_DIR}/external/printf/*
)


set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/firmware.ld)

add_link_options( -mcpu=${MCPU} -nostartfiles -Wl,-T ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})


set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")