// upxer.cpp - Parallel UPX Compressor for libzplay
// Compile: g++ -std=c++17 -O2 upxer.cpp -o upxer.exe

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <algorithm>
using namespace std;
namespace fs = std::filesystem;

std::string g_upxPath;
std::mutex g_printMtx;
std::mutex g_statusMtx;
std::unordered_map<DWORD, size_t> g_statusCount;

// bool compressFile(const fs::path& filePath) {
int compressFile(const fs::path& filePath) {
    std::string cmd = "\"" + g_upxPath + "\" --best --ultra-brute \"" + filePath.string() + "\"";
    // std::string cmd = "\"" + g_upxPath + "\" -9 \"" + filePath.string() + "\"";
    
    STARTUPINFOA si = {sizeof(si)};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi = {};
    char* cmdCopy = _strdup(cmd.c_str());
    
    BOOL ok = CreateProcessA(NULL, cmdCopy, NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    free(cmdCopy);
    
    if (!ok) return false;
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    // return exitCode == 0;
    return exitCode;
}

int main(int argc, char* argv[]) {
    std::string sourceDir = "out";
    std::string outputDir = "out_lite_upx";
    g_upxPath = "E:\\release_ready\\out_lite\\upx-5.1.1-win64\\upx.exe";
    int maxParallel = 4;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-s" && i+1 < argc) sourceDir = argv[++i];
        if (arg == "-o" && i+1 < argc) outputDir = argv[++i];
        if (arg == "-u" && i+1 < argc) g_upxPath = argv[++i];
        if ((arg == "-p" || arg == "--parallel") && i+1 < argc) maxParallel = atoi(argv[++i]);
        if (arg == "-h" || arg == "--help") {
            printf("Usage: upxer [-s source_dir] [-o output_dir] [-u upx_path] [-p N]\n");
            return 0;
        }
    }

    if (!fs::exists(g_upxPath)) {
        printf("ERROR: UPX not found: %s\n", g_upxPath.c_str());
        return 1;
    }

    if (!fs::exists(sourceDir)) {
        printf("ERROR: Source directory not found: %s\n", sourceDir.c_str());
        return 1;
    }

    // Collect files
    std::vector<fs::path> files;
    printf("Scanning %s for .exe and .dll files...\n", sourceDir.c_str());
    
    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        for (auto& c : ext) c = tolower(c);
        if (ext == ".exe" || ext == ".dll") {
            files.push_back(entry.path());
        }
    }

    size_t total = files.size();
    if (total == 0) {
        printf("No compressible files found.\n");
        return 0;
    }

    printf("Found %zu files.\n\n", total);
    fs::path outRoot(outputDir);
    int copyOK = 0, copyFail = 0;
    goto step2;
    // Step 1: Copy all files first
    printf("Step 1: Copying files to %s...\n", outputDir.c_str());
    
    
    for (const auto& src : files) {
        fs::path relPath = fs::relative(src, sourceDir);
        fs::path dst = outRoot / relPath;
        
        std::error_code ec;
        fs::create_directories(dst.parent_path(), ec);
        if (ec) {
            copyFail++;
            continue;
        }
        
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
        if (ec) copyFail++;
        else copyOK++;
    }
    
    printf("Copied: %d OK, %d Fail\n\n", copyOK, copyFail);
    step2:
    // Step 2: Compress files in parallel
    printf("Step 2: Compressing with UPX (--best --ultra-brute)...\n");
    printf("Parallel jobs: %d\n\n", maxParallel);

    auto startTime = std::chrono::steady_clock::now();
    
    std::atomic<size_t> nextIndex{0};
    std::atomic<int> successCount{0};
    std::atomic<int> failCount{0};

    auto worker = [&]() {
        while (true) {
            size_t idx = nextIndex.fetch_add(1);
            if (idx >= total) break;

            fs::path src = files[idx];
            fs::path relPath = fs::relative(src, sourceDir);
            fs::path dst = outRoot / relPath;

            DWORD status = compressFile(dst);

            {
                std::lock_guard<std::mutex> lock(g_statusMtx);
                g_statusCount[status]++;
            }

            if (status == 0) successCount++;
            else failCount++;

            size_t done = successCount + failCount;
            {
                std::lock_guard<std::mutex> lock(g_printMtx);
                size_t statusOccurrence = 0;
                {
                    std::lock_guard<std::mutex> lock2(g_statusMtx);
                    statusOccurrence = g_statusCount[status];
                }
                printf("[%3zu/%3zu] %s - %lu-%zu -> %s\n",
                    done, total,
                    (status == 0) ? "OK" : "FAIL",
                    status, statusOccurrence,
                    (dst.parent_path().string() + "\\" + dst.filename().string()).c_str());
            }
        }
    };

    std::vector<std::thread> threads;
    int numWorkers = std::min(maxParallel, (int)total);
    for (int i = 0; i < numWorkers; i++) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) t.join();

    auto endTime = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(endTime - startTime).count();

    // Calculate size savings
    uintmax_t srcSize = 0, dstSize = 0;
    for (const auto& src : files) {
        fs::path relPath = fs::relative(src, sourceDir);
        fs::path dst = outRoot / relPath;
        if (fs::exists(src)) srcSize += fs::file_size(src);
        if (fs::exists(dst)) dstSize += fs::file_size(dst);
    }

    printf("\n======================================================================\n");
    printf("  COMPRESSION COMPLETE\n");
    printf("======================================================================\n");
    printf("  Elapsed: %.1fs (%.2fmin)\n", elapsed, elapsed/60.0);
    printf("  Total: %-4zu | OK: %-4d | Fail: %-4d\n", total, successCount.load(), failCount.load());
    printf("  Original size:  %.2f MB\n", srcSize / 1024.0 / 1024.0);
    printf("  Compressed size: %.2f MB\n", dstSize / 1024.0 / 1024.0);
    printf("  Saved: %.2f MB (%.1f%%)\n", 
        (srcSize - dstSize) / 1024.0 / 1024.0,
        (1.0 - (double)dstSize / srcSize) * 100.0);
    
    std::vector<std::pair<DWORD, size_t>> statusVec(g_statusCount.begin(), g_statusCount.end());
    std::sort(statusVec.begin(), statusVec.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });
    printf("  Status codes:\n");
    for (const auto& [code, count] : statusVec) {
        printf("    %lu-%zu\n", code, count);
    }
    printf("======================================================================\n");

    return 0;
}
