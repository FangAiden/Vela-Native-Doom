cmake_minimum_required(VERSION 3.10)
project(VelaDoom C)

set(SECTION_START "0x46100000" CACHE STRING "")
set(VELAKIT_DIR "${CMAKE_SOURCE_DIR}/../VelaKit" CACHE PATH "")

set(ARM_TOOLCHAIN_DIR "C:/Program Files/Arm/GNU Toolchain mingw-w64-x86_64-arm-none-eabi/bin")
set(ARM_GCC "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-gcc.exe")
set(ARM_OBJCOPY "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-objcopy.exe")
set(ARM_SIZE "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-size.exe")

file(GLOB DOOM_SOURCES "${CMAKE_SOURCE_DIR}/doomgeneric/*.c")
list(FILTER DOOM_SOURCES EXCLUDE REGEX "icon\\.c$")

set(VELA_SOURCES
    ${CMAKE_SOURCE_DIR}/src/doomgeneric_vela.c
    ${CMAKE_SOURCE_DIR}/src/vela_stubs.c
)

set(ALL_SOURCES ${DOOM_SOURCES} ${VELA_SOURCES})

set(RSP_FILE "${CMAKE_BINARY_DIR}/sources.rsp")
file(WRITE ${RSP_FILE} "")
foreach(src ${ALL_SOURCES})
    file(APPEND ${RSP_FILE} "\"${src}\"\n")
endforeach()

add_custom_target(doom ALL
    COMMAND "${ARM_GCC}"
        -mcpu=cortex-a8 -mfloat-abi=hard -mfpu=neon
        -mthumb
        -std=gnu11
        -nostartfiles
        -nostdinc
        -isystem "${CMAKE_SOURCE_DIR}/include/stubs"
        "-I${CMAKE_SOURCE_DIR}/include"
        "-I${CMAKE_SOURCE_DIR}/doomgeneric"
        "-I${VELAKIT_DIR}/include"
        -include "${CMAKE_SOURCE_DIR}/include/vela_doom.h"
        -DNORMALUNIX
        -DLINUX
        -DDOOMGENERIC_RESX=320
        -DDOOMGENERIC_RESY=200
        -Os
        -ffunction-sections
        -fdata-sections
        -fno-builtin
        -Wl,--gc-sections
        -Wl,--entry=main
        "-Wl,--section-start=.text=${SECTION_START}"
        -Wno-builtin-declaration-mismatch
        -Wno-implicit-function-declaration
        -Wno-int-conversion
        -lgcc
        @${RSP_FILE}
        -o "${CMAKE_BINARY_DIR}/doom.elf"
    COMMAND "${ARM_OBJCOPY}" -O binary
        "${CMAKE_BINARY_DIR}/doom.elf"
        "${CMAKE_BINARY_DIR}/doom.bin"
    COMMAND "${ARM_SIZE}" "${CMAKE_BINARY_DIR}/doom.elf"
    COMMENT "Building DOOM for VelaOS (${SECTION_START})"
)

add_custom_target(loader
    COMMAND "${ARM_GCC}"
        -mcpu=cortex-a8 -mfloat-abi=hard -mfpu=neon
        -mthumb
        -nostartfiles
        "-I${VELAKIT_DIR}/include"
        -DLOAD_ADDRESS=${SECTION_START}
        -DLOAD_SIZE=1048576
        -DBSS_SIZE=0x3A900
        -Wl,--entry=main
        -Wl,--section-start=.text=0x46000000
        -Wno-builtin-declaration-mismatch
        "${CMAKE_SOURCE_DIR}/loader/loader.c"
        -o "${CMAKE_BINARY_DIR}/loader.elf"
    COMMAND "${ARM_OBJCOPY}" -O binary
        "${CMAKE_BINARY_DIR}/loader.elf"
        "${CMAKE_BINARY_DIR}/loader.bin"
    COMMAND "${ARM_SIZE}" "${CMAKE_BINARY_DIR}/loader.elf"
    COMMENT "Building Loader (0x46000000)"
)
