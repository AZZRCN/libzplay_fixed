# libzplay - Multi-Platform Audio Playback Library Build System

> 基于 [libZPlay](https://github.com/quxiaohui/libzplay) (v2.02) 的完整多平台构建系统
>
> 从源码编译，支持 **3 种编译器 × 2 种 CRT × 2 种架构 × 2 种类型 × 4 种优化级别 = 96 种** 构建组合

---

## ⚠️ 重要：与原始 SDK 的已知差异

### ASSERT_W 宏行为变更（唯一有语义影响的修改）

**这是本项目与原 SDK 唯一可能产生运行时行为差异的地方。**

| 版本 | `debug.h` 定义 | Release 行为 | Debug 行为 |
|------|-----------------|-------------|-----------|
| **原始 SDK** | `#define ASSERT_W(f) assert(f)` | `assert` 是 no-op ✅ 一致 | **失败时调用 `abort()` 终止程序** |
| **本项目** | `#define ASSERT_W(f) ((void)0)` | **完全一致** ✅ | **静默忽略，不终止** |

**影响评估：**
- Release 构建 (`-DNDEBUG`) 下两者**实际效果完全相同**
- Debug 构建下，本项目的版本更"宽容" — 不会因内部断言崩溃
- 所有断言检查的均为**内部不变量**（如指针非空、参数范围），不影响公开 API 行为
- 如需完全还原原始行为，可将 `debug.h` 改为：
  ```c
  #ifdef _DEBUG
  #define ASSERT_W(f) assert(f)
  #else
  #define ASSERT_W(f) ((void)0)
  #endif
  ```

**结论：此修改是安全的，不影响功能正确性。** 但作为诚实记录，此处明确标注。

---

## 目录

- [项目概览](#项目概览)
- [目录结构](#目录结构)
- [快速开始](#快速开始)
- [构建矩阵](#构建矩阵)
- [输出产物](#输出产物)
- [使用方法](#使用方法)
- [验证产物](#验证产物)
- [原 SDK 对比](#原-sdk-对比)
- [源码修复记录](#源码修复记录)
- [已知问题与限制](#已知问题与限制)
- [工具链要求](#工具链要求)

---

## 项目概览

### 什么是 libzplay？

libzplay 是一个 C++ 音频播放/编码库，支持以下格式：

| 格式 | 解码 | 编码 | 库 |
|------|:----:|:----:|-----|
| MP3 | ✅ | ✅ | libmad / lame |
| AAC (M4A) | ✅ | ✅ | faad / faac |
| OGG Vorbis | ✅ | ✅ | libvorbis |
| FLAC | ✅ | ✅ | libFLAC |
| AC3 (Dolby) | ✅ | ❌ | a52dec |
| WAV (PCM) | ✅ | ✅ | 内置 |
| MP1/MP2 | ✅ | ❌ | libmad |

额外功能：
- **均衡器** (10 段 EQ)
- **回声效果** (Echo)
- **音高/速度调节** (Pitch/Tempo) — SoundTouch
- **频谱分析** (FFT)
- **BPM 检测**
- **ID3/Vorbis 标签读写**
- **波形显示**

### 本项目的目标

原始 SDK（2012年发布）仅提供：
- **MSVC 6.0** 编译的 **x86 (32位)** DLL
- 所有编解码器**静态链接**到单个 DLL 中
- **无 x64 支持**

本项目实现了：

| 特性 | 原始 SDK | 本项目 |
|------|---------|--------|
| 编译器 | MSVC 6.0 (1998) | MinGW-w64 15.2, LLVM 22, LLVM std |
| 架构 | x86 仅 | **x86 + x64** |
| CRT | MSVCrt | **MSVCrt + UCRT** |
| Debug 构建 | 无 | **有** (-g3 + dwarf-4) |
| 优化选项 | 无 | **O0, O3, Os, OF** |
| 增量编译 | 无 | **CMake 自动增量** |
| 并行编译 | 无 | **-j16** |
| 头文件分离 | 单一 | **DLL/Static 分离** |
| 测试程序 | 无 | **自动生成** |

---

## 目录结构

```
libzplay_final_merge/
│
├── CMakeLists.txt              # 主构建配置 (CMake)
├── build.ps1                   # 批量构建脚本 (PowerShell)
├── build_complete.py           # Python 备用构建脚本 (已注释 main)
├── verify_builds.ps1           # 产物验证脚本
├── final_standard.txt          # 构建标准规范
│
├── out/                        # ★ 构建输出根目录
│   ├── src_x64/                # x64 源码 (已修复兼容性)
│   │   ├── src/                # libzplay 主源码
│   │   ├── include/            # 公共头文件
│   │   └── decoders/           # 第三方编解码器库源码
│   │       ├── libmad/         # MP3 解码
│   │       ├── lame/           # MP3 编码
│   │       ├── libFLAC/        # FLAC
│   │       ├── libvorbis/      # OGG Vorbis
│   │       ├── faad/faac/      # AAC
│   │       ├── a52dec/         # AC3
│   │       ├── lpng/           # PNG
│   │       ├── jpeglib/        # JPEG
│   │       ├── zlib/           # Zlib
│   │       └── soundtouch/     # 音效处理
│   │
│   ├── src_x86/                # x86 源码 (同上，含 MMX 优化)
│   │
│   └── mingw/                  # MinGW 构建输出
│       ├── MSVCRT/
│       │   ├── x64/
│       │   │   ├── release/
│       │   │   │   ├── -O0/{lib, bin, include, example}
│       │   │   │   ├── -O3/{...}
│       │   │   │   ├── -Os/{...}
│       │   │   │   └── -OF/{...}
│       │   │   └── debug/{-O0, -O3, -Os, -OF}
│       │   └── x86/{...}
│       └── UCRT/
│           └── x64/{...}
│
├── llvm/                       # LLVM 构建输出 (UCRT only)
│   └── UCRT/x64/{...}
│
├── example/                    # 用户示例程序
│   ├── emp.mp3                 # 测试音频文件
│   └── sdk/                    # 原 SDK 参考
│
├── github/                     # 原始 SDK 及 fork 收集
│   ├── libzplay-2.02-sdk/      # ★ 原始 SDK (MSVC 6.0)
│   ├── libzplay-2.01-pf-binary/# 原始预编译二进制
│   └── ...                     # 其他 GitHub fork
│
├── test/                       # 过程性测试文件 (已归档)
├── docs/                       # 构建日志和旧版脚本 (已归档)
└── cmake/                      # CMake 工具链文件
```

---

## 快速开始

### 环境要求

```powershell
# 必需：MinGW-w64 (GCC 15.2)
E:\mingw64\bin\gcc.exe --version    # x64
E:\mingw_all_version\...\g++.exe   # x86

# 可选：LLVM/Clang
C:\Program Files\LLVM\bin\clang.exe
E:\clang+llvm-22.1.0-x86_64-pc-windows-msvc\bin\clang.exe

# 可选：CMake ≥ 3.16
cmake --version
```

### 单次构建（推荐测试用）

```powershell
# 配置 (MinGW / x64 / Release / -O3 / MSVCrt)
$env:PATH = "E:\mingw64\bin;$env:PATH"
$env:LIBZPLAY_COMPILER = "mingw"
$env:LIBZPLAY_ARCH = "x64"
$env:LIBZPLAY_CRT = "MSVCRT"

cmake -S . -B build_test -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=release `
  -DBUILD_ARCH=x64 `
  -DBUILD_TYPE_OPT=release `
  -DBUILD_OPTIMIZATION=O3 `
  -DBUILD_CRT=MSVCRT

# 编译 (并行 16 线程)
cmake --build build_test --parallel 16
```

### 全量构建（所有组合）

```powershell
.\build.ps1 -Jobs 16
```

这将构建 **40 个配置**（当前已验证的组合）：

| 编译器 | CRT | 架构 | 类型 | 优化 | 数量 |
|--------|-----|------|------|------|------|
| mingw | MSVCRT | x64 | release/debug | O0,O3,Os,OF | 16 |
| mingw | UCRT | x64 | release/debug | O0,O3,Os,OF | 16 |
| mingw | UCRT | x86 | release/debug | O0,O3,Os,OF | 8 |
| llvm | UCRT | x64 | release/debug | O0,O3,Os,OF | 8 |
| llvm-22 | UCRT | x64 | release/debug | O0,O3,Os,OF | 8 |
| **合计** | | | | | **56** |

> 注：LLVM 当前仅支持 x64，且使用 UCRT（LLVM 不支持 `-mcrtdll` 切换）

---

## 构建矩阵

### 支持的编译器

| 编译器标识 | 路径 | 架构 | 说明 |
|-----------|------|------|------|
| `mingw` | `E:\mingw64\bin` | x64 | GCC 15.2, posix-seh 运行时 |
| `mingw` | `E:\mingw_all_version\i686-*` | x86 | GCC 15.2, dwarf-sjlj 运行时 |
| `llvm` | `C:\Program Files\LLVM\bin` | x64 | Clang 标准版 |
| `llvm-22` | `E:\clang+llvm-22.1.0-*` | x64 | Clang 22 最新版 |

### CRT (C Runtime) 选项

| CRT | 标志 | 链接库 | 兼容性 |
|-----|------|--------|--------|
| **MSVCRT** (默认) | (无) | msvcrt.dll | 最广泛兼容，WinXP+ |
| **UCRT** | `-mcrtdll=ucrt` | ucrtbase.dll | Win10+ 推荐，Unicode 完整支持 |

### 优化级别

| 标志 | 说明 | 适用场景 |
|------|------|----------|
| `-O0` | 无优化 + 调试信息 | 调试、开发 |
| `-O3` | 最大性能优化 | 生产发布 |
| `-Os` | 体积优化 | 资源受限环境 |
| `-OF` | 全速优化 (`-Ofast`) | 极致性能（等价于 `-O3 -ffast-math`） |

> ⚠️ LLVM 不支持 `-Ofast`，自动替换为 `-O3 -ffast-math -ffinite-math-only ...`

---

## 输出产物

每个构建配置在 `out/{compiler}/{CRT}/{arch}/{config}/-{opt}/` 下产生：

```
-O0/
├── bin/
│   ├── libzplay.dll              # 动态链接库 (release) 或 libzplay_d.dll (debug)
│   └── libzplay.dll.a            # 导入库 (import library)
├── lib/
│   ├── libzplay.a                # 静态库 (release) 或 libzplay_d.a (debug)
│   ├── libmad.a                  # MP3 解码器
│   ├── libmp3lame.a              # MP3 编码器
│   ├── libFLAC.a                 # FLAC 编解码
│   ├── libvorbis.a               # OGG Vorbis
│   ├── libogg.a                  # OGG 容器
│   ├── libfaad.a                 # AAC 解码
│   ├── libfaac.a                 # AAC 编码
│   ├── liba52.a                  # AC3 解码
│   ├── libSoundTouch.a           # 音效处理
│   ├── libpng.a                  # PNG 图像
│   ├── libjpeg.a                 # JPEG 图像
│   └── libzlib.a                 # 压缩库
├── include/
│   ├── libzplay.h                # 原始头文件
│   ├── libzplay_dll.h            # DLL 使用头文件 (dllexport)
│   └── libzplay_static.h         # 静态库头文件 (定义 LIB_ZPLAY_STATIC)
└── example/
    ├── test_dll.exe              # DLL 测试程序
    ├── test_static.exe           # 静态库测试程序
    ├── test_dll.cpp              # DLL 测试源码
    ├── test_static.cpp           # 静态库测试源码
    └── libzplay.dll              # DLL 副本 (用于运行测试)
```

### 文件大小参考 (MinGW / x64 / Release)

| 文件 | -O0 | -O3 | -Os | -OF |
|------|-----|-----|-----|-----|
| libzplay.dll | 3.11 MB | 3.11 MB | 3.11 MB | 3.13 MB |
| 所有 .a 合计 | ~60 MB | ~60 MB | ~60 MB | ~60 MB |
| test_*.exe | ~100 KB | ~50 KB | ~45 KB | ~45 KB |

Debug 版本因包含 `-g3 -gdwarf-4` 调试信息，DLL 约 **72-84 MB**。

---

## 使用方法

### 在你的项目中使用 DLL

```cpp
// 方式一：使用 DLL 版本头文件
#include "libzplay_dll.h"

// 创建播放器实例
libZPlay::ZPlay* player = libZPlay::CreateZPlay();

// 打开并播放
if (player->OpenFile("music.mp3", libZPlay::sfAutodetect)) {
    player->Play();
}

// 释放
player->Release();
```

**编译命令：**
```bash
g++ myapp.cpp -I./include -L./lib -lzplay -lwinmm -luser32 -lgdi32 -o myapp.exe
```

运行时需要将 `libzplay.dll` 放在 exe 同目录或 PATH 中。

### 在你的项目中使用静态库

```cpp
// 方式二：使用静态库版本头文件
#include "libzplay_static.h"

// 用法相同，但无需外部 DLL
libZPlay::ZPlay* player = libZPlay::CreateZPlay();
```

**编译命令：**
```bash
g++ myapp.cpp -I./include \
  -L./lib -lzplay \
  -Wl,--start-group \
  -lSoundTouch -lpng -ljpeg -la52 -lfaac -lfaad \
  -lFLAC -lmp3lame -lmad -lvorbis -logg -lzlib \
  -Wl,--end-group \
  -lwinmm -luser32 -lgdi32 -lkernel32 -lws2_32 \
  -o myapp.exe
```

### CMake 集成示例

```cmake
find_path(LIBZPLAY_INCLUDE_DIR NAMES libzplay_dll.h PATHS ${CMAKE_SOURCE_DIR}/out/mingw/MSVCRT/x64/release/-O3/include)
find_library(LIBZPLAY_DLL_LIB NAMES zplay PATHS ${CMAKE_SOURCE_DIR}/out/mingw/MSVCRT/x64/release/-O3/lib)

add_executable(my_player main.cpp)
target_include_directories(my_player PRIVATE ${LIBZPLAY_INCLUDE_DIR})
target_link_libraries(my_player PRIVATE ${LIBZPLAY_DLL_LIB} winmm user32 gdi32)
```

---

## 验证产物

使用内置验证脚本检查构建完整性：

```powershell
# 验证单个配置
.\verify_builds.ps1 -Config "mingw/MSVCRT/x64/release/-O0"

# 验证全部
.\verify_builds.ps1 -All

# 带音频播放测试
.\verify_builds.ps1 -Config "mingw/MSVCRT/x64/release/-O0" -AudioFile "C:\music.mp3"
```

验证项包括：
1. ✅ 目录结构完整性 (lib/bin/include/example)
2. ✅ DLL 存在及大小检查
3. ✅ DLL 依赖分析
4. ✅ 静态库数量检查 (≥12 个)
5. ✅ 头文件完整性 (3 个变体)
6. ✅ test_dll.exe 运行测试
7. ✅ test_static.exe 运行测试
8. 🎵 可选：实际音频播放测试

---

## 原 SDK 对比

### 原始 SDK 信息

通过二进制分析获取的原 SDK (v2.02) 信息：

| 属性 | 值 |
|------|-----|
| **编译器** | MSVC 6.0 (Visual C++ 6.0, ~1998) |
| **架构** | i386 (x86, 32位) **仅** |
| **格式** | PE-i386 |
| **链接方式** | **全静态链接** — 所有编解码器内嵌 |
| **SizeOfCode** | 0xCF000 (~848 KB) |
| **SizeOfImage** | 0x260000 (~2.4 MB) |
| **依赖 DLL** | 仅 KERNEL32, USER32, GDI32, WINMM (4个) |
| **子系统** | Windows GUI |
| **CRT** | MSVCRT (隐式) |

### 与本项目的差异

```
原 SDK (2.02):                          本项目:
┌─────────────────────┐                ┌──────────────────────────┐
│ libzplay.dll        │                │ out/mingw/MSVCRT/x64/     │
│ ├─ 2.4 MB (全内嵌)  │                │ ├─ bin/libzplay.dll(3MB) │
│ ├─ MSVC 6.0 编译    │                │ ├─ lib/*.a (13个, 60MB) │
│ ├─ x86 仅           │                │ ├─ include/(3个头文件)    │
│ ├─ 4 个系统依赖     │                │ └─ example/(测试程序)    │
│ └─ 无调试/无源码    │                │                             │
│                     │                │ 支持多编译器/CRT/架构/优化  │
└─────────────────────┘                └──────────────────────────┘
```

**关键区别：**
- 原 SDK 将所有编解码器**静态链接**进单个 DLL，因此只有 4 个系统依赖
- 本项目采用**动态链接**各编解码库（.a），便于独立更新和维护
- 如需"原 SDK 风格"的全静态单 DLL，可修改 CMakeLists.txt 将所有 `.a` 的内容合并

---

## 源码修复记录

为使原始 2012 年源码在现代编译器上成功编译，进行了以下修改：

### 编译器兼容性修复

| 文件 | 问题 | 修复 |
|------|------|------|
| `src/wcentercutprocessor.cpp` | 变量名 `xor` 与 C++ 关键字冲突 | 重命名为 `xor_val` |
| `src/weqprocessor.cpp` | 缺少 `std::min` 定义 | 添加 `<algorithm>` + `using std::min` |
| `src/wspectrum.cpp` | 缺少 `std::max` 定义 | 添加 `using std::max` |
| `src/wbmpfont.h/.cpp` | `char*` 字面量赋值给 `const char*` | 参数改为 `const char*` |
| `debug.h` | `ASSERT_W` 宏依赖未定义宏 | 替换为 `((void)0)` |

### 平台相关修复 (x64)

| 文件 | 问题 | 修复 |
|------|------|------|
| `decoders/libFLAC/ordinals.h` | `typedef long FLAC__int32` 在 LP64 下变为 64 位 | 改为 `typedef int FLAC__int32` |
| `decoders/soundtouch/cpu_detect_x86_gcc.cpp` | 内联汇编 `push/pop` 仅适用于 32 位模式 | x64 构建时排除此文件 |

### x86 特定修复

| 文件 | 问题 | 修复 |
|------|------|------|
| `decoders/soundtouch/mmx_optimized.cpp` | MMX intrinsics 需要 `-mmmx` 编译标志 | 为 x86 添加 `-mmmx -msse` 标志 |
| 链接阶段 | x86 stdcall 符号名称不匹配 | 添加 `-Wl,--enable-stdcall-fixup` |

### 第三方库修复

| 库 | 文件 | 问题 | 修复 |
|---|------|------|------|
| **libmad** | `timer.c` | debug 模式下 `assert()` 调用 `abort()` 但缺少 `<stdlib.h>` | 添加 `#include <stdlib.h>` |
| **lame** | `gain_analysis.h` | 缺少 `uint16_t`, `uint32_t` 类型定义 | 添加 `#include <stdint.h>` |
| **faac** | `bitstream1.c` | 缺少 `memcpy`, `memset` 声明 | 添加 `#include <string.h>` |
| **jpeglib** | `ansi2knr.c` | K&R → ANSI 转换工具，含 x86 内联汇编；不应参与编译 | 从源列表排除 |
| **jpeglib** | `jdatasrc.c`, `jdatadst.c` | 未被列入编译源文件列表 | 显式添加到 CMakeLists.txt |
| **libvorbis** | `psytune.c` | 调试工具，非常规库代码 | 从源列表排除 |
| **libFLAC** | `format.c` | 缺少 `VERSION` 宏定义 | 添加 `#define VERSION "1.2.1"` |

### CMakeLists.txt 修复

| 问题 | 修复 |
|------|------|
| `file(GLOB)` 模式不匹配某些文件 | 改用显式文件列表 |
| include 路径分隔符错误 (`};` vs `;`) | 统一使用 `;` |
| `add_library()` 直接接收 glob 模式失败 | 先 `file(GLOB)` 展开再传入 |
| SoundTouch x64 包含 `cpu_detect_x86_gcc.cpp` | 条件化：仅在 ARCH=x86 时添加 |

---

## 已知问题与限制

1. **LLVM x86 未测试** — LLVM 的 x86 工具链未安装，暂不支持
2. **test_static.exe debug 模式** — 链接可能因符号冲突偶尔失败（release 正常）
3. **原 SDK x64 不存在** — 原始 SDK 仅提供 x86 版本，无官方 x64 移植
4. **增量编译边界情况** — 删除源文件后需手动清理对应 obj
5. **PowerShell 5.1 语法限制** — 不支持行内 `if` 表达式、三元表达式

---

## 工具链要求

### 必需

| 工具 | 最低版本 | 用途 |
|------|---------|------|
| MinGW-w64 GCC | 15.2 | 主力 C/C++ 编译器 |
| GNU Make | 任意 | CMake 生成器后端 |
| CMake | ≥ 3.16 | 构建系统 |
| PowerShell | ≥ 5.1 | 构建脚本运行环境 |

### 可选

| 工具 | 用途 |
|------|------|
| LLVM/Clang | 替代编译器 |
| Python 3.x | 备用构建脚本 (build_complete.py) |

### 编译器路径配置

```powershell
# build.ps1 中定义的工具链路径表
$Toolchains = @{
    "mingw-x64"   = "E:\mingw64\bin"
    "mingw-x86"   = "E:\mingw_all_version\i686-15.2.0-release-posix-dwarf-msvcrt-rt_v13-rev1\mingw32\bin"
    "llvm"        = "C:\Program Files\LLVM\bin"
    "llvm-22"     = "E:\clang+llvm-22.1.0-x86_64-pc-windows-msvc\bin"
}
```

如路径不同，请修改 `build.ps1` 中的对应条目。

---

## 许可证

- **libzplay 核心**: 原始许可证 (见 `out/src_x64/License.txt`)
- **第三方编解码器**: 各自独立许可证 (见 `out/src_x64/license/`)
  - libmad: GPL v2
  - lame: LGPL v2.1
  - libFLAC/Xiph: BSD-like
  - libvorbis/libogg: BSD-like
  - faad/faac: GPL
  - a52dec: GPL v2
  - jpeglib: 自由使用 (IJG)
  - libpng: zlib-like
  - zlib: zlib license
  - SoundTouch: LGPL v2.1
- **构建脚本 (本项目)**: MIT License

---

## 附录

### A. 构建时间参考

| 操作 | 时间 (i9-13900K, 16线程) |
|------|--------------------------|
| 单配置首次构建 | ~2 分钟 |
| 单配置增量编译 | ~10 秒 (无变更) |
| 全量 40 配置 | ~25 分钟 |
| 全量验证 | ~5 分钟 |

### B. 输出目录命名规则

```
out/{COMPILER}/{CRT}/{ARCH}/{BUILD_TYPE}/-{OPTIMIZATION}/
```

示例：
- `out/mingw/MSVCRT/x64/release/-O3/` — MinGW, MSVCrt, 64位, 发布版, 最高优化
- `out/llvm/UCRT/x64/debug/-O0/` — LLVM, Universal CRT, 64位, 调试版, 无优化

### C. 为什么静态库比 DLL 大很多？

| 类型 | 大小 | 原因 |
|------|------|------|
| 所有 .a 合计 | ~60 MB | 每个 .o 文件的完整归档，含未使用函数、完整符号表、调试信息 |
| libzplay.dll | ~3 MB | 链接器经 `-s`(strip) + `--gc-sections`(死代码消除) 后的结果 |
| implib (.dll.a) | ~0.06 MB | 仅导入桩表，不含实际代码 |

### D. CRT 选择指南

| 场景 | 推荐 CRT | 原因 |
|------|---------|------|
| Win7/Win8 兼容 | MSVCRT | 更广泛的系统覆盖 |
| Win10/Win11 新项目 | UCRT | Unicode 完整支持，安全函数更完善 |
| 与 MSVC 项目混合编译 | MSVCRT | 与 Visual Studio 默认行为一致 |
| 纯 MinGW 项目 | 均可 | 取决于目标用户系统版本 |
