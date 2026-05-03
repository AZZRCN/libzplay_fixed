# libzplay Lite CMake Build Script (Enhanced with Incremental Build)
# Usage:
#   .\build.ps1                          # Build all (smart incremental)
#   .\build.ps1 -Compiler mingw           # mingw only
#   .\build.ps1 -Arch x64                # x64 only
#   .\build.ps1 -CRT UCRT                # UCRT only
#   .\build.ps1 -Optimization O3         # -O3 only
#   .\build.ps1 -Jobs 32                 # parallel jobs
#   .\build.ps1 -Clean                   # Force rebuild
#   .\build.ps1 -CheckOnly               # Show what would be built

param(
    [ValidateSet("mingw", "llvm", "llvm-22", "msvc", "vs-clang")]
    [string]$Compiler = "",

    [ValidateSet("x64", "x86", "arm64")]
    [string]$Arch = "",

    [ValidateSet("MSVCRT", "UCRT")]
    [string]$CRT = "",

    [ValidateSet("release", "debug")]
    [string]$BuildType = "",

    [ValidateSet("O0", "O3", "Os", "OF")]
    [string]$Optimization = "",

    [int]$Jobs = 16,

    [switch]$Clean,

    [switch]$CheckOnly,

    [string]$Generator = ""
)

$ProjectRoot = $PSScriptRoot
if (-not $ProjectRoot) { $ProjectRoot = Get-Location }

$ErrorActionPreference = "Continue"
$script:StartTime = Get-Date
$script:SkippedCount = 0

$script:OriginalPath = $env:PATH

$CMakePath = "C:\Program Files\CMake\bin\cmake.exe"
if (-not (Test-Path $CMakePath)) {
    $CMakePath = "cmake.exe"
}

$Toolchains = @{
    "mingw-x64"     = @{ Bin = "E:\mingw64\bin"; Arch = "x64"; Generator = "MinGW Makefiles"; IsMSVC = $false }
    "mingw-x86"     = @{ Bin = "E:\mingw_all_version\i686-15.2.0-release-posix-dwarf-msvcrt-rt_v13-rev1\mingw32\bin"; Arch = "x86"; Generator = "MinGW Makefiles"; IsMSVC = $false }
    "llvm"          = @{ Bin = "C:\Program Files\LLVM\bin"; Arch = "x64"; Generator = "Ninja"; IsMSVC = $false; UseClangCl = $true }
    "llvm-22"       = @{ Bin = "E:\clang+llvm-22.1.0-x86_64-pc-windows-msvc\bin"; Arch = "x64"; Generator = "Ninja"; IsMSVC = $false; UseClangCl = $true }
    "msvc-x64"      = @{ Bin = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64"; Arch = "x64"; Generator = "Ninja"; IsMSVC = $true; VCVars = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" }
    "msvc-x86"      = @{ Bin = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x86"; Arch = "x86"; Generator = "Ninja"; IsMSVC = $true; VCVars = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"; VCVarsArg = "x64_x86" }
    "msvc-arm64"    = @{ Bin = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.44.35207\bin\Hostx64\arm64"; Arch = "arm64"; Generator = "Ninja"; IsMSVC = $true; VCVars = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"; VCVarsArg = "x64_arm64" }
    "vs-clang-x64"  = @{ Bin = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\Llvm\x64\bin"; Arch = "x64"; Generator = "Ninja"; IsMSVC = $false; IsVSCLang = $true; UseClangCl = $true; VCVars = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" }
    "vs-clang-arm64"= @{ Bin = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\Llvm\x64\bin"; Arch = "arm64"; Generator = "Ninja"; IsMSVC = $false; IsVSCLang = $true; UseClangCl = $true; IsCrossCompile = $true; VCVars = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"; VCVarsArg = "x64_arm64" }
}

$BuildTypes = @("release", "debug")
$Optimizations = @("O0", "O3", "Os", "OF")
$CRTs = @("MSVCRT", "UCRT")

function Test-BuildComplete($outputDir) {
    if (-not (Test-Path $outputDir)) { return $false }
    
    $libDir = "$outputDir\lib"
    $includeDir = "$outputDir\include"
    
    if (-not (Test-Path $libDir)) { return $false }
    $libFiles = Get-ChildItem $libDir -File -ErrorAction SilentlyContinue | Where-Object { $_.Extension -match '\.(a|lib)$' }
    if ($libFiles.Count -lt 10) { return $false }
    
    if (-not (Test-Path $includeDir)) { return $false }
    $headerFiles = Get-ChildItem $includeDir -File -ErrorAction SilentlyContinue | Where-Object { $_.Extension -eq '.h' }
    if ($headerFiles.Count -lt 1) { return $false }
    
    return $true
}

function Get-OutputDir($tcName, $crt, $btype, $opt) {
    $tc = $Toolchains[$tcName]
    $compParts = $tcName -split "-"
    $compShort = $compParts[0]
    if ($compParts.Count -gt 2) {
        $compShort = $compParts[0] + "-" + $compParts[1]
    } elseif ($compParts.Count -eq 2 -and $compParts[1] -notmatch '^(x64|x86|arm64)$') {
        $compShort = $tcName
    }
    $archDir = $tc.Arch
    return "out/$compShort/$crt/$archDir/$btype/-$opt"
}

function Clear-MSVCEnv {
    $msvcPaths = @(
        "C:\Program Files\Microsoft Visual Studio",
        "C:\Program Files (x86)\Microsoft Visual Studio",
        "C:\Program Files (x86)\Windows Kits"
    )
    
    @("INCLUDE", "LIB", "LIBPATH", "CL", "_CL_", "LINK", "_LINK_", 
      "VCINSTALLDIR", "VSINSTALLDIR", "WindowsSdkDir", "WindowsSDKVersion") | ForEach-Object {
        [Environment]::SetEnvironmentVariable($_, $null, "Process")
    }
    
    if ($script:OriginalPath) {
        $cleanPath = ($script:OriginalPath -split ";" | Where-Object {
            $p = $_.Trim()
            -not ($msvcPaths | Where-Object { $p -like "$_*" -or $p -like "$($_)\*" })
        }) -join ";"
        [Environment]::SetEnvironmentVariable("PATH", $cleanPath, "Process")
    }
}

function Invoke-VcVars($vcvarsPath, $vcvarsArg) {
    Clear-MSVCEnv
    
    $tmpFile = "$env:TEMP\vcvars_$([System.Diagnostics.Process]::GetCurrentProcess().Id)_$(Get-Random).tmp"
    $batFile = "$env:TEMP\vcvars_loader_$(Get-Random).bat"

    $argStr = if ($vcvarsArg) { $vcvarsArg } else { "" }
    $batContent = "@echo off`r`ncall `"$vcvarsPath`" $argStr >nul 2>&1`r`nset > `"$tmpFile`"`r`nexit /b 0"
    $batContent | Out-File -FilePath $batFile -Encoding ASCII -Force

    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = "cmd.exe"
    $psi.Arguments = "/c `"$batFile`""
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardOutput = [System.IO.StringWriter]::new()
    $psi.RedirectStandardError = [System.IO.StringWriter]::new()
    
    $proc = [System.Diagnostics.Process]::Start($psi)
    $proc.WaitForExit()

    Start-Sleep -Milliseconds 100

    if (Test-Path $tmpFile) {
        Get-Content $tmpFile | ForEach-Object {
            if ($_ -match "^([^=]+)=(.*)$") {
                [Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], "Process")
            }
        }
        Remove-Item $tmpFile -ErrorAction SilentlyContinue
        Remove-Item $batFile -ErrorAction SilentlyContinue
        return $true
    }

    Remove-Item $batFile -ErrorAction SilentlyContinue
    return $false
}

function Build-One($tcName, $crt, $btype, $opt) {
    $tc = $Toolchains[$tcName]
        $label = "$tcName/$crt/$btype/-$opt"
        $safeLabel = $label -replace '[\\/:*?"<>|]', '_'
        $buildDir = "build_cmake/${safeLabel}"
        $outputDir = Get-OutputDir $tcName $crt $btype $opt

        Write-Host "  Building: $label" -NoNewline

        if (-not $Clean) {
        if (Test-BuildComplete $outputDir) {
            Write-Host " [SKIP]" -ForegroundColor Green
            $script:SkippedCount++
            return @{ Success = $true; Skipped = $true }
        }
    } else {
        Remove-Item -Recurse -Force $buildDir -ErrorAction SilentlyContinue
        Remove-Item -Recurse -Force $outputDir -ErrorAction SilentlyContinue
    }

    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null

    $actualGenerator = if ($tc.Generator) { $tc.Generator } else { $Generator }
    if (-not $actualGenerator) { $actualGenerator = "MinGW Makefiles" }

    $compShort = ($tcName -split "-")[0]
    $compParts = $tcName -split "-"
    $compilerName = $compParts[0]
    if ($compParts.Count -gt 2 -or ($compParts.Count -eq 2 -and $compParts[1] -notmatch '^(x64|x86|arm64)$')) {
        $compilerName = $tcName -replace '-(x64|x86|arm64)$', ''
    }

    $env:LIBZPLAY_COMPILER = $compilerName
    $env:LIBZPLAY_ARCH = $tc.Arch
    $env:LIBZPLAY_CRT = $crt

    $cmakeArgs = @(
        "-S", $ProjectRoot,
        "-B", $buildDir,
        "-G", $actualGenerator,
        "-DCMAKE_BUILD_TYPE=$btype",
        "-DBUILD_ARCH=$($tc.Arch)",
        "-DBUILD_TYPE_OPT=$btype",
        "-DBUILD_OPTIMIZATION=$opt",
        "-DBUILD_CRT=$crt"
    )

    if ($tc.IsMSVC) {
        $msvcBase = "C:/Program Files/Microsoft Visual Studio/2022/Professional/VC/Tools/MSVC/14.44.35207/bin"
        $hostArch = "Hostx64"
        $targetArch = if ($tc.Arch -eq "x86") { "x86" } elseif ($tc.Arch -eq "arm64") { "arm64" } else { "x64" }
        $clPath = "`"$msvcBase/$hostArch/$targetArch/cl.exe`""
        $cmakeArgs += "-DCMAKE_C_COMPILER=$clPath"
        $cmakeArgs += "-DCMAKE_CXX_COMPILER=$clPath"

        $vcvarsArg = if ($tc.VCVarsArg) { $tc.VCVarsArg } else { "" }
        $cmakeArgStr = $cmakeArgs -join " "

        $msvcRoot = "C:\Program Files\Microsoft Visual Studio\2022\Professional"
        $msvcVer = "14.44.35207"
        $sdkDir = "${env:ProgramFiles(x86)}\Windows Kits\10"
        $sdkVer = "10.0.26100.0"

        $includeDirs = @(
            "$msvcRoot\VC\Tools\MSVC\$msvcVer\include",
            "$msvcRoot\VC\Tools\MSVC\$msvcVer\atlmfc\include",
            "$sdkDir\Include\$sdkVer\ucrt",
            "$sdkDir\Include\$sdkVer\um",
            "$sdkDir\Include\$sdkVer\shared",
            "$sdkDir\Include\$sdkVer\winrt",
            "$sdkDir\Include\$sdkVer\cppwinrt"
        )
        [Environment]::SetEnvironmentVariable("INCLUDE", ($includeDirs -join ";"), "Process")

        $libDirs = @(
            "$msvcRoot\VC\Tools\MSVC\$msvcVer\lib\$targetArch",
            "$msvcRoot\VC\Tools\MSVC\$msvcVer\atlmfc\lib\$targetArch",
            "$sdkDir\Lib\$sdkVer\ucrt\$targetArch",
            "$sdkDir\Lib\$sdkVer\um\$targetArch"
        )
        [Environment]::SetEnvironmentVariable("LIB", ($libDirs -join ";"), "Process")

        $argsFile = "$env:TEMP\cmake_args_$([System.Diagnostics.Process]::GetCurrentProcess().Id)_$(Get-Random).txt"
        $cmakeArgStr | Out-File -FilePath $argsFile -Encoding ASCII -Force

        $sdkBinDir = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0"
        $hostArch = "x64"
        $targetSdkBin = if ($tc.Arch -eq "x86") { "x86" } elseif ($tc.Arch -eq "x64") { "x64" } elseif ($tc.Arch -eq "arm64") { $hostArch }

        $includeStr = $includeDirs -join ";"
        $libStr = $libDirs -join ";"

        $batContent = @"
@echo off
setlocal
set "PATH=E:\ninja;$sdkBinDir\$targetSdkBin;%PATH%"
set "INCLUDE=$includeStr"
set "LIB=$libStr"
set /p CMAKE_ARGS=<"$argsFile"
"$CMakePath" %CMAKE_ARGS%
if %ERRORLEVEL% NEQ 0 (
    echo CMAKE_CONFIG_FAILED
    exit /b 1
)
"$CMakePath" --build "$buildDir" --parallel $Jobs
if %ERRORLEVEL% NEQ 0 (
    echo BUILD_FAILED
    exit /b 1
)
"@
        $batFile = "$env:TEMP\msvc_build_$([System.Diagnostics.Process]::GetCurrentProcess().Id)_$(Get-Random).bat"
        $batContent | Out-File -FilePath $batFile -Encoding ASCII -Force

        $psi = New-Object System.Diagnostics.ProcessStartInfo
        $psi.FileName = "cmd.exe"
        $psi.Arguments = "/c `"$batFile`""
        $psi.UseShellExecute = $false
        $psi.CreateNoWindow = $false
        $psi.RedirectStandardOutput = $false
        $psi.RedirectStandardError = $false

        $proc = [System.Diagnostics.Process]::Start($psi)
        $proc.WaitForExit()
        $output = ""
        $errors = ""

        Remove-Item $batFile -ErrorAction SilentlyContinue
        Remove-Item $argsFile -ErrorAction SilentlyContinue

        if ($proc.ExitCode -ne 0) {
            Write-Host "  [FAIL]" -ForegroundColor Red
            return @{ Success = $false; Skipped = $false }
        }
    }
    elseif ($tc.UseClangCl) {
        $clangClPath = "$($tc.Bin)\clang-cl.exe"
        $cmakeArgs += "-DCMAKE_C_COMPILER=`"$clangClPath`""
        $cmakeArgs += "-DCMAKE_CXX_COMPILER=`"$clangClPath`""
        $cmakeArgs += "-DCMAKE_LINKER=`"$($tc.Bin)\lld-link.exe`""
        
        if ($tc.IsCrossCompile -and $tc.Arch -eq "arm64") {
            $cmakeArgs += "-DCMAKE_C_FLAGS=--target=arm64-pc-windows-msvc"
            $cmakeArgs += "-DCMAKE_CXX_FLAGS=--target=arm64-pc-windows-msvc"
        }

        $msvcRoot = "C:\Program Files\Microsoft Visual Studio\2022\Professional"
        $msvcVer = "14.44.35207"
        $sdkDir = "${env:ProgramFiles(x86)}\Windows Kits\10"
        $sdkVer = "10.0.26100.0"
        $targetArch = $tc.Arch

        $includeDirs = @(
            "$msvcRoot\VC\Tools\MSVC\$msvcVer\include",
            "$sdkDir\Include\$sdkVer\ucrt",
            "$sdkDir\Include\$sdkVer\um",
            "$sdkDir\Include\$sdkVer\shared"
        )
        $libDirs = @(
            "$msvcRoot\VC\Tools\MSVC\$msvcVer\lib\$targetArch",
            "$sdkDir\Lib\$sdkVer\ucrt\$targetArch",
            "$sdkDir\Lib\$sdkVer\um\$targetArch"
        )

        $argsFile = "$env:TEMP\cmake_args_$([System.Diagnostics.Process]::GetCurrentProcess().Id)_$(Get-Random).txt"
        ($cmakeArgs -join " ") | Out-File -FilePath $argsFile -Encoding ASCII -Force

        $sdkBinDir = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0"
        $targetSdkBin = if ($tc.Arch -eq "x86") { "x86" } else { "x64" }

        $batContent = @"
@echo off
setlocal
set "PATH=E:\ninja;$sdkBinDir\$targetSdkBin;$($tc.Bin);%PATH%"
set "INCLUDE=$(($includeDirs -join ';'))"
set "LIB=$(($libDirs -join ';'))"
set /p CMAKE_ARGS=<"$argsFile"
"$CMakePath" %CMAKE_ARGS%
if %ERRORLEVEL% NEQ 0 (
    echo CMAKE_CONFIG_FAILED
    exit /b 1
)
"$CMakePath" --build "$buildDir" --parallel $Jobs
if %ERRORLEVEL% NEQ 0 (
    echo BUILD_FAILED
    exit /b 1
)
"@
        $batFile = "$env:TEMP\clangcl_build_$([System.Diagnostics.Process]::GetCurrentProcess().Id)_$(Get-Random).bat"
        $batContent | Out-File -FilePath $batFile -Encoding ASCII -Force

        $psi = New-Object System.Diagnostics.ProcessStartInfo
        $psi.FileName = "cmd.exe"
        $psi.Arguments = "/c `"$batFile`""
        $psi.UseShellExecute = $false
        $psi.CreateNoWindow = $false

        $proc = [System.Diagnostics.Process]::Start($psi)
        $proc.WaitForExit()

        Remove-Item $batFile -ErrorAction SilentlyContinue
        Remove-Item $argsFile -ErrorAction SilentlyContinue

        if ($proc.ExitCode -ne 0) {
            Write-Host "  [FAIL]" -ForegroundColor Red
            return @{ Success = $false; Skipped = $false }
        }
    }
    elseif ($tc.IsVSCLang -and $tc.VCVars) {
        $clangPath = "$($tc.Bin)\clang.exe"
        $clangxxPath = "$($tc.Bin)\clang++.exe"
        $cmakeArgs += "-DCMAKE_C_COMPILER=`"$clangPath`""
        $cmakeArgs += "-DCMAKE_CXX_COMPILER=`"$clangxxPath`""
        if ($tc.IsCrossCompile) {
            $cmakeArgs += "-DCMAKE_SYSTEM_NAME=Windows"
            $cmakeArgs += "-DCMAKE_SYSTEM_PROCESSOR=ARM64"
            $cmakeArgs += "-DCMAKE_C_FLAGS=--target=arm64-pc-windows-msvc"
            $cmakeArgs += "-DCMAKE_CXX_FLAGS=--target=arm64-pc-windows-msvc"
        }

        $vcvarsArg = if ($tc.VCVarsArg) { $tc.VCVarsArg } else { "" }
        $cmakeArgStr = $cmakeArgs -join " "

        $argsFile = "$env:TEMP\cmake_args_$([System.Diagnostics.Process]::GetCurrentProcess().Id)_$(Get-Random).txt"
        $cmakeArgStr | Out-File -FilePath $argsFile -Encoding ASCII -Force

        $batContent = @"
@echo off
setlocal
call "$($tc.VCVars)" $vcvarsArg >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo VCVARS_FAILED
    exit /b 1
)
set /p CMAKE_ARGS=<"$argsFile"
"$CMakePath" %CMAKE_ARGS%
if %ERRORLEVEL% NEQ 0 (
    echo CMAKE_CONFIG_FAILED
    exit /b 1
)
"$CMakePath" --build "$buildDir" --parallel $Jobs
if %ERRORLEVEL% NEQ 0 (
    echo BUILD_FAILED
    exit /b 1
)
"@
        $batFile = "$env:TEMP\vsclang_build_$([System.Diagnostics.Process]::GetCurrentProcess().Id)_$(Get-Random).bat"
        $batContent | Out-File -FilePath $batFile -Encoding ASCII -Force

        $psi = New-Object System.Diagnostics.ProcessStartInfo
        $psi.FileName = "cmd.exe"
        $psi.Arguments = "/c `"$batFile`""
        $psi.UseShellExecute = $false
        $psi.CreateNoWindow = $false
        $psi.RedirectStandardOutput = $false
        $psi.RedirectStandardError = $false

        $proc = [System.Diagnostics.Process]::Start($psi)
        $proc.WaitForExit()
        $output = ""
        $errors = ""

        Remove-Item $batFile -ErrorAction SilentlyContinue
        Remove-Item $argsFile -ErrorAction SilentlyContinue

        if ($proc.ExitCode -ne 0) {
            Write-Host "  [FAIL]" -ForegroundColor Red
            return @{ Success = $false; Skipped = $false }
        }
    }
    else {
        # Set compiler for MinGW/LLVM
        if ($tc.Bin) {
            $gccPath = "$($tc.Bin)\gcc.exe"
            $gxxPath = "$($tc.Bin)\g++.exe"
            $clangPath = "$($tc.Bin)\clang.exe"
            $clangxxPath = "$($tc.Bin)\clang++.exe"
            
            if (Test-Path $gxxPath) {
                $cmakeArgs += "-DCMAKE_C_COMPILER=`"$gccPath`""
                $cmakeArgs += "-DCMAKE_CXX_COMPILER=`"$gxxPath`""
            } elseif (Test-Path $clangxxPath) {
                $cmakeArgs += "-DCMAKE_C_COMPILER=`"$clangPath`""
                $cmakeArgs += "-DCMAKE_CXX_COMPILER=`"$clangxxPath`""
            }
        }
        
        $cmakeOutput = & $CMakePath @cmakeArgs 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "  [FAIL] CMake" -ForegroundColor Red
            return @{ Success = $false; Skipped = $false }
        }

        & $CMakePath --build $buildDir --parallel $Jobs 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            if (Test-BuildComplete $outputDir) {
                Write-Host "  [OK] (with warnings)" -ForegroundColor Yellow
                return @{ Success = $true; Skipped = $false }
            } else {
                Write-Host "  [FAIL]" -ForegroundColor Red
                return @{ Success = $false; Skipped = $false }
            }
        }
    }

    if (Test-BuildComplete $outputDir) {
        Write-Host "  [OK]" -ForegroundColor Green
        return @{ Success = $true; Skipped = $false }
    } else {
        Write-Host "  [WARN] Incomplete" -ForegroundColor Yellow
        return @{ Success = $true; Skipped = $false }
    }
}

$results = @()
$total = 0
$success = 0
$failed = 0

Write-Host ""
Write-Host ("=" * 60)
Write-Host "  libzplay Lite Build System v2.1"
Write-Host ("=" * 60)
Write-Host "  Date: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
$modeStr = if($Clean){'Full Rebuild'}else{'Incremental'}
if($CheckOnly){$modeStr += ' (Dry Run)'}
Write-Host "  Mode: $modeStr"
Write-Host "  Note: FFT, BPM, EQ, Echo removed"
Write-Host ""

foreach ($tcName in $Toolchains.Keys) {
    $tc = $Toolchains[$tcName]
    
    # Extract compiler name for filtering (e.g., "vs-clang" from "vs-clang-x64")
    $compParts = $tcName -split "-"
    if ($compParts.Count -eq 1) {
        $compShort = $tcName
    } elseif ($compParts.Count -ge 2 -and $compParts[1] -match '^(x64|x86|arm64)$') {
        $compShort = $compParts[0]
    } else {
        $compShort = "$($compParts[0])-$($compParts[1])"
    }

    if ($Compiler -and $compShort -ne $Compiler) { continue }
    if ($Arch -and $tc.Arch -ne $Arch) { continue }

    foreach ($crts in $CRTs) {
        if ($CRT -and $crts -ne $CRT) { continue }

        # Skip mingw-x86+UCRT (not supported by the compiler)
        if ($tc.Arch -eq "x86" -and $compShort -eq "mingw" -and $crts -eq "UCRT") {
            continue
        }

        foreach ($bt in $BuildTypes) {
            if ($BuildType -and $bt -ne $BuildType) { continue }

            foreach ($o in $Optimizations) {
                if ($Optimization -and $o -ne $Optimization) { continue }

                $total++
                $label = "$tcName/$crts/$bt/-$o"
                
                if ($CheckOnly) {
                    $outputDir = Get-OutputDir $tcName $crts $bt $o
                    $isComplete = Test-BuildComplete $outputDir
                    $status = if ($isComplete) { "[OK] Complete" } else { "[--] Missing" }
                    Write-Host "  $status $label -> $outputDir"
                    if ($isComplete) { $script:SkippedCount++ } else { $failed++ }
                    continue
                }
                
                $result = Build-One $tcName $crts $bt $o
                $results += [PSCustomObject]@{
                    Label = $label
                    OK = $result.Success
                    Skipped = $result.Skipped
                }
                if ($result.Success) { $success++ } else { $failed++ }
            }
        }
    }
}

$elapsed = (Get-Date) - $script:StartTime
Write-Host ""
Write-Host ("=" * 60)
Write-Host "  BUILD SUMMARY"
Write-Host ("=" * 60)
Write-Host "  Time: $($elapsed.ToString('mm\:ss')) | Total: $total | OK: $success | Skip: $script:SkippedCount | Fail: $failed"
if ($total -gt 0) {
    $rate = [math]::Round((($success + $script:SkippedCount)/$total)*100, 1)
    Write-Host "  Completion: $rate%"
}

if (-not $CheckOnly -and $failed -gt 0) {
    Write-Host ""
    Write-Host "  Failed configurations:" -ForegroundColor Red
    foreach ($r in $results) {
        if (-not $r.OK -and -not $r.Skipped) {
            Write-Host "    [FAIL] $($r.Label)" -ForegroundColor Red
        }
    }
}

Write-Host ("=" * 60)
Write-Host "  Tip: Use -CheckOnly to see what needs building"
Write-Host "  Tip: Use -Clean to force rebuild all configurations"
Write-Host ("=" * 60)
Write-Host ""

if ($CheckOnly) {
    Write-Host "This was a dry run. No builds were performed." -ForegroundColor Yellow
    Write-Host "Already built: $script:SkippedCount, Need building: $failed"
    exit 0
}

if ($failed -eq 0 -and $total -gt 0) { 
    Write-Host "All builds completed successfully!" -ForegroundColor Green
    exit 0 
} elseif ($success -gt 0) { 
    Write-Host "Some builds failed. Check output above." -ForegroundColor Yellow
    exit 1 
} else { 
    exit 1 
}
