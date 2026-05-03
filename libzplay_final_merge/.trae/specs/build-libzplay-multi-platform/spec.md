# libzplay 多平台构建规范

## Why

libzplay 是一个 2012 年的音频播放库项目，目前存在以下问题：

1. 官方 SDK 和源码仅支持 x86 架构编译
2. 项目资源分散在多个文件夹中（github 收集了多个分支，libzplay\_x64 editon 是 AIGC 的 64 位尝试版本）
3. 缺乏统一的构建标准和输出目录结构
4. 需要根据 final\_standard.txt 要求，使用多种编译器（mingw/llvm/cl\_msvc）构建多架构（x86/x64）、多配置（release/debug）、多优化级别（-O0/-O3/-Os/-OF）的构建产物

## What Changes

* **构建系统整理**：分析 github 文件夹内所有 libzplay 相关项目的构建方式（VC++ 项目、CMake 等）

* **x86 源码整理**：从官方 libzplay-2.02-src 和 github 项目整理出完整的 x86 源代码

* **x64 源码验证**：验证 libzplay\_x64 editon 的 64 位修改是否可行，必要时进行修复

* **多编译器支持**：配置 mingw（多版本）、llvm、cl\_msvc 三种编译器工具链

* **标准化输出**：按照 final\_standard.txt 要求的目录结构输出所有构建产物

* **资源清理**：整理完成后删除或归档原始文件夹，只保留整理后的源码和构建产物

## Impact

* **受影响 specs**：无（这是首个 spec）

* **受影响代码**：

  * `github/` 文件夹内的所有项目（需要分析和提取有用内容）

  * `libzplay_x64 editon/` 文件夹（需要验证和修复）

  * `example/` 文件夹（需要更新为使用新构建的库）

  * 新增 `out/` 文件夹作为标准化输出目录

## ADDED Requirements

### Requirement: 构建产物多样性

The system SHALL 提供以下维度的构建产物自由组合：

* 编译器：mingw（多个版本）/ llvm / cl\_msvc

* 架构：x64 / x86

* 配置：release / debug

* 优化级别：-O0 / -O3 / -Os / -OF

* 库类型：静态库 / 导入库 / 动态运行时库（dll）/ dll 头文件 / lib 头文件

* CRT 类型：UCRT / MSVCRT（仅 MSVC 编译器）

* 符号保留：原符号（不修饰）/ 标准修饰

#### Scenario: 任意组合合法性

* **WHEN** 用户选择任意上述选项组合

* **THEN** 构建系统都能生成合法的构建产物，且产物符合 final\_standard.txt 规定的目录结构

### Requirement: 输出目录结构

The system SHALL 按照以下深度结构组织输出：

```
out/
├── mingw/ (或 llvm/ 或 cl_msvc/)
│   ├── x64/ (或 x86/)
│   │   ├── release/ (或 debug/)
│   │   │   ├── -O0/ (或 -O3/ -OF/ -Os/)
│   │   │   │   ├── lib/
│   │   │   │   ├── bin/
│   │   │   │   └── include/
│   │   │   └── ...
│   │   └── ...
│   └── ...
```

### Requirement: 源代码整理

The system SHALL 整理出两套完整的源代码：

* **x86 源代码**：基于官方 libzplay-2.02-src 和 github 项目验证可用的 x86 版本

* **x64 源代码**：基于 libzplay\_x64 editon 整理并验证可用的 x64 版本

### Requirement: github 资源利用

The system SHALL 充分利用 github 文件夹内的内容：

* 分析每个项目的构建配置和修改

* 提取有价值的补丁和修改（如 CMake 支持、64 位修复等）

* 合并多个项目的优点到整理后的源码中

### Requirement: 清理策略

The system SHALL 在任务结束后：

* 删除 github/ 文件夹（内容已整合到源码中）

* 删除 libzplay\_x64 editon/ 文件夹（内容已整合到源码中）

* 保留 example/ 文件夹但更新为使用新构建的库

* 保留 out/ 文件夹作为最终构建产物

## MODIFIED Requirements

无（这是初始规范）

## REMOVED Requirements

无（这是初始规范）

## 技术约束

### 可用编译器

1. **MinGW 多版本**：`E:\mingw_all_version`（包含多个 mingw 版本，包括 x86 和 x64）
2. **MinGW64**：`E:\mingw64\bin`
3. **LLVM**：`E:\clang+llvm-22.1.0-x86_64-pc-windows-msvc`
4. **LLVM 系统版**：`C:\Program Files\LLVM`
5. **CMake**：`C:\Program Files\CMake`
6. **Ninja**：`E:\ninja`

### 构建目标

libzplay 库包含以下组件：

* 核心库：libzplay.cpp 及相关源文件

* 解码器：libmad（MP3）、libvorbis（OGG）、libFLAC、faad（AAC）、liba52（AC3）等

* 编码器：lame（MP3）、vorbisenc（OGG）、faac（AAC）、FLAC 等

* 工具库：jpeglib、libpng、zlib 等

* DSP 处理：SoundTouch（可选）

### 构建模式

* **动态库**：生成 libzplay.dll + 导入库（.lib/.a）+ 头文件

* **静态库**：生成 libzplay.lib/.a（包含完整实现）+ 头文件

* **PF 版本**：无专利版本（不含 MP3 和 AAC 支持）

## 成功标准

1. out/ 文件夹包含所有要求的构建产物组合
2. 每个构建产物都能正常使用（通过示例程序验证）
3. 源码整理完成，原始文件夹已清理
4. 提供完整的构建文档和使用说明

