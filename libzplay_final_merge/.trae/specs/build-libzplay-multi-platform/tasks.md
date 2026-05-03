# Tasks

- [x] **Task 1: 分析 github 文件夹内所有项目**
  - [x] 列出 github 文件夹内所有子项目
  - [x] 分析每个项目的构建系统（VC++/CMake/其他）
  - [x] 识别每个项目的修改和改进点
  - [x] 提取有价值的补丁和配置文件

- [x] **Task 2: 验证 libzplay_x64 editon 的可行性**
  - [x] 检查 x64 版本的源码修改
  - [x] 尝试使用可用编译器构建 x64 版本
  - [x] 记录构建过程中的问题和解决方案
  - [x] 确定是否需要修复以及修复方案

- [x] **Task 3: 整理 x86 源代码**
  - [x] 以 libzplay-2.02-src 为基础
  - [x] 合并 github 项目中的有用修改
  - [x] 添加 CMake 构建支持（参考 libzplay-main）
  - [x] 验证 x86 源码的完整性

- [x] **Task 4: 整理 x64 源代码**
  - [x] 基于验证后的 x64 修改
  - [x] 修复 64 位编译问题
  - [x] 添加 CMake 构建支持
  - [x] 验证 x64 源码的完整性

- [x] **Task 5: 配置多编译器工具链**
  - [x] 配置 MinGW 多版本工具链（x86 和 x64）
  - [x] 配置 LLVM/Clang 工具链
  - [x] 配置 MSVC 工具链（cl_msvc）
  - [x] 创建统一的 CMake 工具链配置文件

- [x] **Task 6: 实现自动化构建脚本**
  - [x] 创建 Python 构建脚本支持所有编译器组合
  - [x] 实现所有优化级别（-O0/-O3/-Os/-OF）的构建
  - [x] 实现动态库和静态库的构建
  - [x] 实现 PF 版本（无专利）的构建

- [x] **Task 7: 执行完整构建矩阵**
  - [x] 构建所有 mingw 版本（x86/x64, release/debug, 所有优化级别）
  - [ ] 构建所有 llvm 版本（x86/x64, release/debug, 所有优化级别）
  - [ ] 构建所有 cl_msvc 版本（x86/x64, release/debug, 所有优化级别，UCRT/MSVCRT）
  - [x] 按照 final_standard.txt 结构输出到 out/ 文件夹

- [x] **Task 8: 验证构建产物**
  - [x] 抽样测试不同构建产物（使用 example 项目）
  - [x] 验证 DLL 的架构正确性（使用 objdump）
  - [ ] 验证导入库和静态库的正确性
  - [ ] 记录验证结果

- [ ] **Task 9: 清理和整理**
  - [ ] 将整理后的 x86 源码存放到 out/src_x86/
  - [ ] 将整理后的 x64 源码存放到 out/src_x64/
  - [ ] 删除 github/ 文件夹
  - [ ] 删除 libzplay_x64 editon/ 文件夹
  - [ ] 更新 example/ 文件夹使用新构建的库

- [ ] **Task 10: 生成文档**
  - [ ] 生成构建说明文档
  - [ ] 生成使用文档
  - [ ] 生成 API 文档（从源码生成）
  - [ ] 记录所有已知的构建问题和解决方案

# Task Dependencies
- Task 2 依赖于 Task 1（需要先了解 github 项目内容）
- Task 3 和 Task 4 依赖于 Task 1 和 Task 2
- Task 5 依赖于 Task 3 和 Task 4（需要整理后的源码）
- Task 6 依赖于 Task 5
- Task 7 依赖于 Task 6
- Task 8 依赖于 Task 7
- Task 9 依赖于 Task 8 和 Task 3、Task 4
- Task 10 可以在 Task 3、Task 4 完成后并行进行
