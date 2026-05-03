# libzplay 工作区发布包

**版本**: 2.02  
**发布日期**: 2026-05-02  
**构建配置**: 136个 (全部通过验证)

---

## 概述

本工作区包含 libzplay 音频库的完整源码、构建系统、编译缓存及多编译器多架构的构建产物。

**支持格式**: MP3, OGG Vorbis, FLAC, WAV/PCM, AAC, AC3

---

## 目录结构

```
out_fine/
├── src_x64/                 # x64架构源码
├── src_x86/                 # x86架构源码
├── build_cmake/             # CMake构建缓存 (保留用于增量构建)
├── out/                     # 编译产物输出目录
│   ├── msvc/                # MSVC 2022 编译输出
│   ├── mingw/               # MinGW GCC 编译输出
│   ├── llvm/                # LLVM Clang 编译输出
│   ├── llvm-22/             # LLVM 22.1.0 编译输出
│   └── vs-clang/            # VS-Clang 19.1 编译输出
├── build.ps1                # 构建脚本
├── builder.cpp/exe          # 并行构建工具
├── verifier.cpp/exe         # 输出验证工具
├── CMakeLists.txt           # CMake配置
└── BUILD_STATUS.md          # 构建状态报告
```

---

## 编译器支持矩阵

| 编译器 | x64 | x86 | arm64 | UCRT | MSVCRT |
|--------|:---:|:---:|:-----:|:----:|:------:|
| MSVC 2022 (19.44) | ✅ | ✅ | ✅ | ✅ | ✅ |
| MinGW GCC | ✅ | ✅ | - | ✅ | ✅ |
| LLVM Clang | ✅ | - | - | ✅ | ✅ |
| VS-Clang 19.1 | ✅ | - | ✅ | ✅ | ✅ |

**总计**: 136个配置

---

## 构建输出结构

每个配置输出目录包含:

```
{compiler}/{CRT}/{arch}/{type}/-{opt}/
├── bin/        # DLL文件
├── lib/        # 静态库和导入库 (.lib/.a)
├── obj/        # 中间目标文件 (.obj)
├── dbg/        # 调试符号 (.pdb/.debug)
├── include/    # 头文件
└── example/    # 示例程序
```

---

## 使用构建脚本

```powershell
# 构建所有配置
.\build.ps1

# 构建指定编译器和架构
.\build.ps1 -Compiler msvc -Arch x64

# 完全重新构建
.\build.ps1 -Clean

# 验证输出
.\verifier.exe
```

### 参数说明

| 参数 | 可选值 |
|------|--------|
| `-Compiler` | `mingw`, `llvm`, `msvc`, `vs-clang` |
| `-Arch` | `x64`, `x86`, `arm64` |
| `-CRT` | `UCRT`, `MSVCRT` |
| `-BuildType` | `release`, `debug` |
| `-Optimization` | `O0`, `O3`, `Os`, `OF` |
| `-Clean` | 强制重新构建 |

---

## 优化级别

| 选项 | 说明 |
|------|------|
| `-O0` | 无优化，用于调试 |
| `-O3` | 最高优化，用于发布 |
| `-Os` | 优化体积 |
| `-OF` | 快速数学优化 |

---

## 依赖的编译器路径

构建脚本 `build.ps1` 中配置的编译器路径:

- **MSVC**: `C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.44.35207`
- **MinGW-x64**: `E:\mingw64\bin`
- **MinGW-x86**: `E:\mingw_all_version\i686-15.2.0-release-posix-dwarf-msvcrt-rt_v13-rev1\mingw32\bin`
- **LLVM**: `C:\Program Files\LLVM\bin`
- **LLVM-22**: `E:\clang+llvm-22.1.0-x86_64-pc-windows-msvc\bin`
- **VS-Clang**: `C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\Llvm\x64\bin`

---

## 许可证

GPL v2 或更高版本。详见 `src_x64/license/` 目录。
