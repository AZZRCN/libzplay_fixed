# libzplay Lite 工作区发布包

**版本**: 2.02 Lite  
**发布日期**: 2026-05-02  
**状态**: ⚠️ 开发中 - 保守方案（保留内部实现，仅API层隐藏）

---

## 概述

libzplay Lite 是 libzplay 音频库的精简版本，专注于核心音频播放功能，移除了非必要特性以减小体积。

**与完整版区别**：
- ❌ 移除：FFT频谱分析、BPM节拍检测、均衡器、回声效果
- ❌ 移除：图片支持 (JPEG/PNG)、音频编码器
- ✅ 保留：MP3/OGG/FLAC/AAC/AC3解码播放
- ✅ 保留：音量控制、播放速度/音调调整、反向播放、循环播放

---

## 目录结构

```
out_lite/
├── src_x64/                 # x64架构源码
├── src_x86/                 # x86架构源码
├── build_cmake/             # CMake构建缓存
├── output/                  # 编译产物输出目录
├── build.ps1                # 构建脚本
├── builder.cpp/exe          # 并行构建工具
├── verifier.cpp/exe         # 输出验证工具
└── CMakeLists.txt           # CMake配置
```

---

## 编译器支持

| 编译器 | x64 | x86 | arm64 |
|--------|:---:|:---:|:-----:|
| MSVC 2022 | ✅ | ✅ | ✅ |
| MinGW GCC | ✅ | ✅ | - |
| LLVM Clang | ✅ | - | - |
| VS-Clang | ✅ | - | ✅ |

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

---

## 技术说明

### 当前状态
采用**保守方案**：保留完整内部实现代码，仅在公共API层隐藏已移除功能。这确保了稳定性，但DLL体积与完整版相近。

### 已移除的公共API
- `EnableEqualizer`, `SetEqualizerParam`, `GetEqualizerParam`
- `EnableEcho`, `SetEchoParam`, `GetEchoParam`
- `GetFFTData`, `DrawFFTGraphOnHDC`, `DrawFFTGraphOnHWND`
- `SetFFTGraphParam`, `GetFFTGraphParam`
- `DetectBPM`, `DetectFileBPM`, `DetectFileBPMW`

### 已删除的实现文件
- `wbpmdetect1.cpp`, `wbpmdetect2.cpp`, `wbpmdetect3.cpp`
- `wspectrum.cpp`
- 编码器相关代码

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
