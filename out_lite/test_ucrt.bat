@echo off
set LIBZPLAY_COMPILER=mingw
set LIBZPLAY_ARCH=x64
set LIBZPLAY_CRT=UCRT
cd /d e:\libzplay_final_merge\out_lite
rmdir /s /q build_cmake\test_ucrt 2>nul
cmake -B build_cmake\test_ucrt -S . -G "MinGW Makefiles" -DCMAKE_C_COMPILER=E:\mingw64\bin\gcc.exe -DCMAKE_CXX_COMPILER=E:\mingw64\bin\g++.exe -DBUILD_ARCH=x64 -DBUILD_TYPE_OPT=release -DBUILD_OPTIMIZATION=O3 -DBUILD_CRT=UCRT 2>&1
echo ===BUILDING===
cmake --build build_cmake\test_ucrt --parallel 16 2>&1
echo EXIT CODE: %ERRORLEVEL%
if exist out\mingw\UCRT\x64\release\-O3\bin\ (
  echo ===BIN===
  dir out\mingw\UCRT\x64\release\-O3\bin\
) else (
  echo NO BIN DIR
)
if exist out\mingw\UCRT\x64\release\-O3\lib\ (
  echo ===LIB COUNT===
  dir /b out\mingw\UCRT\x64\release\-O3\lib\ | find /c /v ""
  dir out\mingw\UCRT\x64\release\-O3\lib\
) else (
  echo NO LIB DIR
)
