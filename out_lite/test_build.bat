@echo off
set LIBZPLAY_COMPILER=mingw
set LIBZPLAY_ARCH=x64
set LIBZPLAY_CRT=MSVCRT
cd /d e:\libzplay_final_merge\out_lite
rmdir /s /q build_cmake\test_mingw_debug 2>nul
cmake -B build_cmake\test_mingw_debug -S . -G "MinGW Makefiles" -DCMAKE_C_COMPILER=E:\mingw64\bin\gcc.exe -DCMAKE_CXX_COMPILER=E:\mingw64\bin\g++.exe -DBUILD_ARCH=x64 -DBUILD_TYPE_OPT=debug -DBUILD_OPTIMIZATION=O0 -DBUILD_CRT=MSVCRT
cmake --build build_cmake\test_mingw_debug --parallel 16
echo EXIT CODE: %ERRORLEVEL%
if exist out\mingw\MSVCRT\x64\debug\-O0\bin\ (
  dir out\mingw\MSVCRT\x64\debug\-O0\bin\
) else (
  echo NO BIN DIR
)
