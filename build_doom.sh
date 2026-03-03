#!/bin/bash

set -e

ARM_GCC="C:/Program Files/Arm/GNU Toolchain mingw-w64-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc.exe"
ARM_OBJCOPY="C:/Program Files/Arm/GNU Toolchain mingw-w64-x86_64-arm-none-eabi/bin/arm-none-eabi-objcopy.exe"
ARM_SIZE="C:/Program Files/Arm/GNU Toolchain mingw-w64-x86_64-arm-none-eabi/bin/arm-none-eabi-size.exe"

BUILD_DIR="build"
SECTION_START="0x46100000"

mkdir -p "$BUILD_DIR"

SOURCES=()

for f in doomgeneric/*.c; do
    if [[ ! "$f" =~ icon\.c$ ]]; then
        SOURCES+=("$f")
    fi
done

SOURCES+=(src/doomgeneric_vela.c src/vela_stubs.c)

"$ARM_GCC" \
    -mcpu=cortex-a8 -mfloat-abi=hard -mfpu=neon \
    -mthumb \
    -std=gnu11 \
    -nostartfiles \
    -nostdinc \
    -isystem "include/stubs" \
    -I"include" \
    -I"doomgeneric" \
    -I"../VelaKit/include" \
    -include "include/vela_doom.h" \
    -DNORMALUNIX \
    -DLINUX \
    -DDOOMGENERIC_RESX=320 \
    -DDOOMGENERIC_RESY=200 \
    -Os \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -Wl,--gc-sections \
    -Wl,--entry=main \
    -Wl,--section-start=.text=$SECTION_START \
    -Wno-builtin-declaration-mismatch \
    -Wno-implicit-function-declaration \
    -Wno-int-conversion \
    -lgcc \
    "${SOURCES[@]}" \
    -o "$BUILD_DIR/doom.bin"

"$ARM_OBJCOPY" -O binary "$BUILD_DIR/doom.bin" "$BUILD_DIR/doom.as"

"$ARM_SIZE" "$BUILD_DIR/doom.bin"
