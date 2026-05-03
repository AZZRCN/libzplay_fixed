@echo off
cd /d e:\libzplay_final_merge\out_lite
cl /EHsc /std:c++17 /O2 builder.cpp /Fe:builder.exe
echo EXIT CODE: %ERRORLEVEL%
if exist builder.exe (
  echo Builder compiled successfully!
  builder.exe --help
) else (
  echo Failed to compile builder!
)
