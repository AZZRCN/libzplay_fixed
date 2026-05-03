# out_lite 项目进度报告

**更新时间**: 2026-05-02  
**状态**: 开发中 - 源码编译阶段

---

## 已完成工作

### 1. 临时文件清理 ✅
- 删除了所有临时脚本（.bat, .py）
- 删除了测试程序（auto_play, play_test, fft_demo等）
- 删除了构建日志

### 2. 构建工具修复 ✅
- **build.ps1**: 
  - 添加 `UseClangCl = $true` 到 vs-clang 配置
  - 添加 arm64 交叉编译支持 (`--target=arm64-pc-windows-msvc`)
  - 添加 mingw 显式编译器路径设置
- **verifier.cpp/exe**: 创建了输出验证工具

### 3. Lite理念彻底贯彻 - 源码清理 ✅

#### 已删除的功能模块：
| 模块 | 文件 | 说明 |
|------|------|------|
| FFT频谱分析 | wfft.cpp/h, wspectrum.cpp/h | 频谱可视化 |
| BPM节拍检测 | wbpmdetect.cpp/h (1/2/3) | 节拍检测 |
| 均衡器 | weqprocessor.cpp/h | 音频均衡 |
| 回声效果 | wechoprocessor.cpp/h | 回声处理 |
| CenterCut | wcentercutprocessor.cpp/h | 人声分离 |
| BMP字体 | wbmpfont.cpp/h | 位图字体 |
| 编码器基类 | wencoder.cpp/h | 编码器接口 |
| AAC编码器 | waacencoder.cpp/h | AAC输出 |
| FLAC编码器 | wflacencoder.cpp/h | FLAC输出 |
| MP3编码器 | wmp3encoder.cpp/h | MP3输出 |
| Vorbis编码器 | wvorbisencoder.cpp/h | OGG输出 |
| Wave编码器 | wwaveencoder.cpp/h | WAV输出 |
| JPEG库 | decoders/jpeglib/ | 图片解码 |
| PNG库 | decoders/lpng/ | PNG解码 |
| FAAC库 | decoders/faac/ | AAC编码 |
| LAME库 | decoders/lame/ | MP3编码 |

#### 已修改的核心文件：
- **wmp3x.h**: 移除编码器、CenterCut、FFT相关成员变量和函数声明
- **wmp3x.cpp**: 移除大量编码器、PNG/JPEG相关代码实现

### 4. CMakeLists.txt重写 ✅
- 所有解码器从源码编译（不依赖预编译库）
- 支持的解码器：AAC(faad), AC3(liba52), FLAC(libFLAC), MP3(libmad), Vorbis(libogg+libvorbis)
- SoundTouch从源码编译
- 添加 `-w` 标志禁用警告
- 添加 `HAVE_MEMCPY` 定义修复faad兼容性

### 5. 示例程序清理 ✅
- 删除 decode_to_aac/flac/mp3/ogg.cpp（使用已移除功能）
- 删除 display_fft.cpp, detect_bpm.cpp

---

## 当前问题

### 正在解决：faad codebook头文件路径
```
fatal error: codebook/hcb_1.h: No such file or directory
```
**已添加**: `${DECODERS_DIR}/faad/libfaad` 到 include 路径  
**状态**: 待验证

---

## 保留功能（Lite版核心）

### 音频格式支持
- ✅ MP3 解码 (libmad)
- ✅ OGG/Vorbis 解码 (libvorbis)
- ✅ FLAC 解码 (libFLAC)
- ✅ AAC 解码 (faad2)
- ✅ AC3 解码 (liba52)
- ✅ PCM/WAV 解码
- ✅ WAVEIN 录音输入

### 音频处理
- ✅ SoundTouch: 变速/变调/慢放快放
- ✅ 音量控制
- ✅ 播放速度调整
- ✅ 音调调整
- ✅ 反向播放
- ✅ 循环播放
- ✅ 混合声道

---

## 目录结构（当前）
```
out_lite/
├── src_x64/               # x64架构源码
│   ├── src/              # libzplay核心源码
│   ├── include/          # 头文件
│   ├── example/          # 示例程序
│   ├── decoders/         # 解码器源码
│   │   ├── faad/         # AAC解码器
│   │   ├── a52dec/       # AC3解码器
│   │   ├── libFLAC/      # FLAC解码器
│   │   ├── libmad/       # MP3解码器
│   │   ├── libogg/       # OGG容器
│   │   ├── libvorbis/    # Vorbis解码
│   │   └── soundtouch/   # 音频处理器
│   └── CMakeLists.txt    # 构建配置
├── src_x86/              # x86架构源码（同上）
├── build_cmake/          # CMake构建缓存
├── build.ps1             # PowerShell构建脚本
├── builder.cpp/exe       # 并行构建工具
├── verifier.cpp/exe      # 输出验证工具
└── README.md             # 说明文档
```

---

## 下一步计划

1. [ ] 验证faad codebook路径修复是否有效
2. [ ] 完成mingw编译测试
3. [ ] 测试其他编译器（MSVC, LLVM, vs-clang）
4. [ ] 运行完整构建并使用UPX --beat优化
5. [ ] 运行verifier验证输出完整性
6. [ ] 移动到release_ready目录

---

## 技术决策记录

### 为什么保留SoundTouch？
SoundTouch提供核心播放功能：变速、变调、慢放/快放。这些是音频播放器的基础功能，不属于"高级特效"。

### 为什么从源码编译解码器？
预编译的.lib文件是MSVC格式，MinGW无法直接链接。从源码编译确保跨编译器兼容性。

### 为什么不是"衔尾蛇"式？
项目依赖外部编译器（MSVC, MinGW, LLVM等），需要用户自行安装。这是"工作区发布"，不是独立发行包。
