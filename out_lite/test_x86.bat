@echo off
set LIBZPLAY_COMPILER=mingw
set LIBZPLAY_ARCH=x86
set LIBZPLAY_CRT=MSVCRT
cd /d e:\libzplay_final_merge\out_lite
rmdir /s /q build_cmake\test_x86 2>nul
cmake -B build_cmake\test_x86 -S . -G "MinGW Makefiles" -DCMAKE_C_COMPILER=E:\mingw_all_version\i686-15.2.0-release-posix-dwarf-msvcrt-rt_v13-rev1\mingw32\bin\gcc.exe -DCMAKE_CXX_COMPILER=E:\mingw_all_version\i686-15.2.0-release-posix-dwarf-msvcrt-rt_v13-rev1\mingw32\bin\g++.exe -DBUILD_ARCH=x86 -DBUILD_TYPE_OPT=release -DBUILD_OPTIMIZATION=O3 -DBUILD_CRT=MSVCRT 2>&1
echo ===BUILDING===
cmake --build build_cmake\test_x86 --parallel 16 2>&1 | findstr /i "error Error FAILED fatal"
echo EXIT CODE: %ERRORLEVEL%
if exist out\mingw\MSVCRT\x86\release\-O3\bin\ (
  echo ===BIN===
  dir out\mingw\MSVCRT\x86\release\-O3\bin\
) else (
  echo NO BIN DIR
)
if exist out\mingw\MSVCRT\x86\release\-O3\lib\ (
  echo ===LIB===
  dir out\mingw\MSVCRT\x86\release\-O3\lib\
) else (
  echo NO LIB DIR
)
