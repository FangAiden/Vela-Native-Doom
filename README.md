# VelaDoom

VelaDoom 是一个面向 VelaOS 的 DOOM 移植工程，基于 `doomgeneric`，并通过 `VelaKit` 完成平台适配。

## 仓库结构

- `doomgeneric/`：DOOM 核心源码
- `src/`：Vela 平台适配代码
- `include/`：头文件与 libc stub
- `scripts/deploy.py`：构建 + 部署 + 运行脚本
- `build_doom.sh`：快速构建脚本
- `wad/`：本地 IWAD 资源目录
- `docs/`：辅助文档与符号数据

## 环境要求

- Windows + PowerShell
- Arm GNU Toolchain（`arm-none-eabi-gcc`）
- `../VelaKit` 目录存在（与本仓库同级）
- Python 3
- `adb`

## WAD 下载与放置

本仓库不包含商业版 DOOM 资源。

推荐使用开源 IWAD：

1. 从 Freedoom 官方页面下载 `freedoom1.wad`：`https://freedoom.github.io/download.html`
2. 放到本仓库路径：

```text
wad/freedoom1.wad
```

PowerShell 示例：

```powershell
New-Item -ItemType Directory -Force wad | Out-Null
Copy-Item "C:\Downloads\freedoom1.wad" "wad\freedoom1.wad"
```

当前项目默认已对齐到该文件名：

- 本地部署源文件：`wad/freedoom1.wad`（`scripts/deploy.py`）
- 设备端路径：`/data/freedoom1.wad`（`scripts/deploy.py`）
- 运行时 `-iwad` 参数：`/data/freedoom1.wad`（`src/doomgeneric_vela.c`）

## 第三方许可证说明（Freedoom）

`freedoom1.wad` 是开源资源，但再分发时仍需保留 Freedoom 的版权与许可证声明（BSD 3-Clause）。

如果你在仓库或发布包中包含 `wad/freedoom1.wad`，建议在发布说明或 `THIRD_PARTY_NOTICES` 文件中附上 Freedoom 上游许可信息。

参考：

- `https://freedoom.github.io/about.html`
- `https://github.com/freedoom/freedoom`

## 构建

### 方式一：CMake

```powershell
cmake -S . -B build
cmake --build build --target doom
```

如需构建 loader（前提是存在 `loader/loader.c`）：

```powershell
cmake --build build --target loader
```

### 方式二：脚本

```bash
bash build_doom.sh
```

构建输出位于 `build/`（例如 `doom.bin` 与 `doom.as`）。

## 部署与运行

```powershell
python scripts/deploy.py
```

也可以分步执行：

```powershell
python scripts/deploy.py --build
python scripts/deploy.py --deploy
python scripts/deploy.py --run
```

默认 ADB 设备为 `emulator-5554`，可在 `scripts/deploy.py` 中修改。

## 备注

- `.gitignore` 当前规则会忽略 `wad/*.wad`，但保留 `wad/freedoom1.wad` 可提交。
- 请不要提交商业版 DOOM WAD 文件。
- 就当前代码组成而言，项目许可证建议使用 `GPL-3.0-or-later`。
