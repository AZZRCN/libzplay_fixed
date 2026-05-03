# libzplay 构建状态

**构建日期**: 2026-05-01  
**版本**: 2.02

---

## 构建统计

| 指标 | 数值 |
|------|------|
| 总配置数 | 136 |
| 成功 | 136 |
| 失败 | 0 |
| 跳过 | 1 (mingw-x86+UCRT) |

---

## 编译器支持矩阵

### MSVC 2022 (19.44)

| 架构 | CRT | Release | Debug |
|------|-----|:-------:|:-----:|
| x64 | UCRT | ✅ | ✅ |
| x64 | MSVCRT | ✅ | ✅ |
| x86 | UCRT | ✅ | ✅ |
| x86 | MSVCRT | ✅ | ✅ |
| arm64 | UCRT | ✅ | ✅ |
| arm64 | MSVCRT | ✅ | ✅ |

### MinGW GCC

| 架构 | CRT | Release | Debug |
|------|-----|:-------:|:-----:|
| x64 | UCRT | ✅ | ✅ |
| x64 | MSVCRT | ✅ | ✅ |
| x86 | MSVCRT | ✅ | ✅ |
| x86 | UCRT | ⏭️ | ⏭️ |

### LLVM Clang

| 架构 | CRT | Release | Debug |
|------|-----|:-------:|:-----:|
| x64 | UCRT | ✅ | ✅ |
| x64 | MSVCRT | ✅ | ✅ |

### VS-Clang (Clang 19.1)

| 架构 | CRT | Release | Debug |
|------|-----|:-------:|:-----:|
| x64 | UCRT | ✅ | ✅ |
| x64 | MSVCRT | ✅ | ✅ |
| arm64 | UCRT | ✅ | ✅ |
| arm64 | MSVCRT | ✅ | ✅ |

---

## 库文件

| 文件 | 类型 | 大小 |
|------|------|------|
| libzplay.dll | 动态链接库 | ~2MB |
| libzplay.dll.lib | 导入库 | ~50KB |
| libzplay_static.lib | 静态库 | ~50MB |

---

## 已修复的Bug

| Bug | 文件 | 修复 |
|-----|------|------|
| 未定义行为 | zlib/inflate.c | `-1L << 16` → `-(1L << 16)` |
| 逻辑错误 | wmp3x.cpp | `=` → `==` |
| 未初始化变量 | wmp3x.cpp | 添加初始化 |
| 内存泄漏 | 多个头文件 | 添加虚析构函数 |
| 冗余代码 | VbrTag.c | 移除无效检查 |

---

## 测试结果

### DLL测试 ✅

```
ZPlay instance created successfully!
Version: 202
File opened: emp.mp3
Sample rate: 48000 Hz
Channels: 2
Duration: 00:02:10.680
```

### 静态库测试 ✅

```
ZPlay instance created successfully!
Version: 202
File opened: emp.mp3
Sample rate: 48000 Hz
Channels: 2
Duration: 00:02:10.680
可执行文件大小: 2.5MB (无需DLL)
```
