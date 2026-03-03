#!/usr/bin/env python3

import subprocess
import sys
import os
import argparse
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
BUILD_DIR = os.path.join(PROJECT_DIR, "build")
VELAKIT_DIR = os.path.join(PROJECT_DIR, "..", "VelaKit")

ARM_TOOLCHAIN = "C:/Program Files/Arm/GNU Toolchain mingw-w64-x86_64-arm-none-eabi/bin"
ARM_GCC = os.path.join(ARM_TOOLCHAIN, "arm-none-eabi-gcc.exe")
ARM_OBJCOPY = os.path.join(ARM_TOOLCHAIN, "arm-none-eabi-objcopy.exe")
GENERATE_SH = os.path.join(VELAKIT_DIR, "scripts", "generate_sh.py")

ADB_DEVICE = "emulator-5554"
LOADER_ADDR = "0x46000000"
DOOM_ADDR = "0x46100000"


def run(cmd, **kwargs):
    env = os.environ.copy()
    env["MSYS_NO_PATHCONV"] = "1"
    return subprocess.run(cmd, capture_output=True, text=True, env=env, **kwargs)


def adb(*args, timeout=30):
    cmd = ["adb", "-s", ADB_DEVICE] + list(args)
    return run(cmd, timeout=timeout)


def adb_shell(cmd, timeout=30):
    return adb("shell", cmd, timeout=timeout)


def build():
    os.makedirs(BUILD_DIR, exist_ok=True)

    print("[BUILD] Compiling DOOM...")
    rsp_file = os.path.join(BUILD_DIR, "sources.rsp")
    if not os.path.exists(rsp_file):
        print(f"  ERROR: {rsp_file} not found. Run cmake first.")
        sys.exit(1)

    doom_bin = os.path.join(BUILD_DIR, "doom.bin")
    doom_as = os.path.join(BUILD_DIR, "doom.as")

    r = run([
        ARM_GCC,
        "-mcpu=cortex-a8", "-mfloat-abi=hard", "-mfpu=neon", "-mthumb",
        "-std=gnu11", "-nostartfiles", "-nostdinc",
        "-isystem", os.path.join(PROJECT_DIR, "include/stubs"),
        f"-I{os.path.join(PROJECT_DIR, 'include')}",
        f"-I{os.path.join(PROJECT_DIR, 'doomgeneric')}",
        f"-I{os.path.join(VELAKIT_DIR, 'include')}",
        "-include", os.path.join(PROJECT_DIR, "include/vela_doom.h"),
        "-DNORMALUNIX", "-DLINUX",
        "-DDOOMGENERIC_RESX=320", "-DDOOMGENERIC_RESY=200",
        "-Os", "-ffunction-sections", "-fdata-sections", "-fno-builtin",
        "-Wl,--gc-sections", "-Wl,--entry=main",
        f"-Wl,--section-start=.text={DOOM_ADDR}",
        "-Wno-builtin-declaration-mismatch",
        "-Wno-implicit-function-declaration",
        "-Wno-int-conversion",
        "-lgcc",
        f"@{rsp_file}",
        "-o", doom_bin,
    ], timeout=300)

    if r.returncode != 0:
        print(f"  FAILED: {r.stderr}")
        sys.exit(1)

    run([ARM_OBJCOPY, "-O", "binary", doom_bin, doom_as])
    doom_size = os.path.getsize(doom_as)
    print(f"  doom.as: {doom_size:,} bytes")

    print("[BUILD] Compiling Loader...")
    loader_bin = os.path.join(BUILD_DIR, "loader.bin")
    loader_as = os.path.join(BUILD_DIR, "loader.as")

    r = run([
        ARM_GCC,
        "-mcpu=cortex-a8", "-mfloat-abi=hard", "-mfpu=neon", "-mthumb",
        "-nostartfiles",
        f"-I{os.path.join(VELAKIT_DIR, 'include')}",
        f"-DLOAD_ADDRESS={DOOM_ADDR}",
        "-DLOAD_SIZE=1048576",
        "-DBSS_SIZE=0x3A900",
        "-Wl,--entry=main",
        f"-Wl,--section-start=.text={LOADER_ADDR}",
        "-Wno-builtin-declaration-mismatch",
        os.path.join(PROJECT_DIR, "loader/loader.c"),
        "-o", loader_bin,
    ], timeout=60)

    if r.returncode != 0:
        print(f"  FAILED: {r.stderr}")
        sys.exit(1)

    run([ARM_OBJCOPY, "-O", "binary", loader_bin, loader_as])
    loader_size = os.path.getsize(loader_as)
    print(f"  loader.as: {loader_size:,} bytes")

    print("[BUILD] Generating mw commands...")
    loader_sh = os.path.join(BUILD_DIR, "loader.sh")
    run([sys.executable, GENERATE_SH, loader_as, "--addr", LOADER_ADDR, "-o", loader_sh])
    print("  Done.")


def deploy():
    doom_as = os.path.join(BUILD_DIR, "doom.as")
    wad_file = os.path.join(PROJECT_DIR, "wad", "freedoom1.wad")
    loader_sh = os.path.join(BUILD_DIR, "loader.sh")

    for f in [doom_as, wad_file, loader_sh]:
        if not os.path.exists(f):
            print(f"  ERROR: {f} not found")
            sys.exit(1)

    r = adb_shell("echo ok")
    if r.returncode != 0:
        print(f"  ERROR: Cannot reach {ADB_DEVICE}")
        sys.exit(1)

    print("[DEPLOY] Pushing doom.bin...")
    r = adb("push", doom_as, "/data/doom.bin")
    print(f"  {r.stdout.strip()}")

    print("[DEPLOY] Pushing freedoom1.wad...")
    r = adb("push", wad_file, "/data/freedoom1.wad")
    print(f"  {r.stdout.strip()}")

    r = adb_shell("ls -l /data/doom.bin; ls -l /data/freedoom1.wad")
    for line in r.stdout.strip().split("\n"):
        print(f"  {line.strip()}")

    print("[DEPLOY] Writing loader to memory...")
    with open(loader_sh) as f:
        cmds = [l.strip() for l in f if l.strip()]

    failures = 0
    for i, cmd in enumerate(cmds):
        try:
            r = adb_shell(cmd, timeout=10)
            if r.returncode != 0:
                failures += 1
        except subprocess.TimeoutExpired:
            failures += 1
        if (i + 1) % 100 == 0:
            print(f"  {i+1}/{len(cmds)} mw commands sent")

    print(f"  {len(cmds)} commands sent, {failures} failures")

    r = adb_shell(f"xd {LOADER_ADDR} 16")
    print(f"  Loader at {LOADER_ADDR}: {r.stdout.strip()[:60]}...")


def execute():
    print("[RUN] Executing DOOM via loader...")
    print(f"  exec {LOADER_ADDR} + 1 (Thumb)")
    print("  Press Ctrl+C to stop")
    print()

    try:
        proc = subprocess.Popen(
            ["adb", "-s", ADB_DEVICE, "shell", f"exec 0x46000001"],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, bufsize=1,
        )
        for line in iter(proc.stdout.readline, ""):
            print(line, end="")
    except KeyboardInterrupt:
        print("\n[RUN] Interrupted by user")
        proc.kill()


def main():
    parser = argparse.ArgumentParser(description="VelaDoom deploy tool")
    parser.add_argument("--build", action="store_true", help="Only build")
    parser.add_argument("--deploy", action="store_true", help="Only deploy")
    parser.add_argument("--run", action="store_true", help="Only run")
    args = parser.parse_args()

    do_all = not (args.build or args.deploy or args.run)

    if do_all or args.build:
        build()
    if do_all or args.deploy:
        deploy()
    if do_all or args.run:
        execute()


if __name__ == "__main__":
    main()
