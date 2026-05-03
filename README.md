# libzplay (fine & lite)
# libzplay (fine & lite)

> 面向 2026 年编译器的 libzplay 现代化重构。
> Modernized refactoring of libzplay, tailored for 2026 compilers.
> 两个分支，一种态度：不妥协的轻量。
> Two branches, one attitude: uncompromising lightweight.
> 
> 在现代 C/C++ 生态里，找一个“包含头文件就能出声”的音频库几乎是一种奢望。多数方案要么依赖庞大的底层框架，要么裹挟着不需要的臃肿模块。
> In the modern C/C++ ecosystem, finding an audio library that "just works with a single include" is almost a luxury. Most solutions either rely on massive underlying frameworks or come bundled with unwanted bloat.
> 
> 如果你只是想在几十 MB 甚至上百 MB 的项目里，播放一个格式工厂转换出来的、几十 KB 大小的提示音——你不需要 FFT 频谱分析，不需要均衡器，你只需要它能出声，且不带来额外的集成负担。
> If you just want to play a tiny, tens-of-KB notification sound (converted by Format Factory) in a project tens or even hundreds of MB in size—you don't need FFT spectrum analysis, you don't need an EQ. You just need it to make a sound, without bringing extra integration baggage.
> 
> 基于这个近乎偏执的需求，我对 Zoran Cindori 的原版 libzplay 动了刀子。
> Driven by this almost paranoid requirement, I took a scalpel to Zoran Cindori's original libzplay.

### 两个平行宇宙
### Two Parallel Universes

本仓库提供两个完全独立的构建版本，按需取用：
This repo offers two completely independent build versions, take what you need:

**▶ `fine` (全功能版)**
**▶ `fine` (Full-featured)**

原汁原味的现代复刻。没有砍掉任何功能，没有修改任何业务逻辑。
An authentic modern replica. No features removed, no business logic altered.
它的存在只有一个目的：**救活**。将原本无法在现代编译器下通过的古老代码，重写 CMake 构建系统，剥离过时的 x86 内联汇编，使其在 MSVC 2022 / Clang 以及 x64 / ARM64 架构下重获新生。
Its sole purpose is **resurrection**. Taking ancient code that fails on modern compilers, rewriting the CMake build system, stripping obsolete x86 inline assembly, and giving it new life under MSVC 2022 / Clang and x64 / ARM64 architectures.

**▶ `lite` (极致精简版)**
**▶ `lite` (Ultra-minimalist)**

外科手术式的暴力裁剪。剔除了一切与“纯粹解码播放”无关的冗余（频谱、节拍、特效、图片解析、编码器）。
Surgical, brutal trimming. Excised everything unrelated to "pure decoding and playback" (spectrum, beats, effects, image parsing, encoders).
原版 SDK 3.1MB → 精简后 0.7MB → 经 UPX 极限压缩后，核心 DLL 仅剩 **400KB**。
Original SDK 3.1MB → Trimmed to 0.7MB → Post extreme UPX compression, the core DLL is merely **400KB**.
集成代价趋近于零。
The integration cost approaches zero.
