// libzplay_parallel_builder.cpp
// C++ Parallel Build System - MSVC compatible
// Compile: cl /EHsc /std:c++17 /O2 builder.cpp /Fe:builder.exe

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max

#ifndef FOREGROUND_CYAN
#define FOREGROUND_CYAN (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#endif

#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

struct BuildConfig {
    std::string toolchain;
    std::string crt;
    std::string buildType;
    std::string optimization;
};

struct BuildResult {
    std::atomic<bool> started{false};
    std::atomic<bool> finished{false};
    std::atomic<int> exitCode{-1};
    std::string output;
    std::string label;
};

class ParallelBuilder {
public:
    explicit ParallelBuilder(int maxParallel, int jobsPerTask)
        : m_maxParallel(maxParallel), m_jobsPerTask(jobsPerTask) {}

    void addConfig(const BuildConfig& cfg) {
        m_configs.push_back(cfg);
    }

    void runAll() {
        auto startTime = std::chrono::steady_clock::now();
        size_t total = m_configs.size();

        printHeader(total);

        std::mutex mtx;
        std::condition_variable cv;
        int activeCount = 0;
        int completedCount = 0;
        int successCount = 0;
        int failCount = 0;

        std::vector<BuildResult> results(total);
        for (size_t i = 0; i < total; i++) {
            results[i].label = formatLabel(m_configs[i]);
        }

        auto worker = [&]() {
            while (true) {
                size_t idx = SIZE_MAX;
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    if (m_nextIndex >= total) break;
                    cv.wait(lock, [&] { 
                        return activeCount < m_maxParallel || m_nextIndex >= total; 
                    });
                    if (m_nextIndex >= total) break;
                    idx = m_nextIndex++;
                    activeCount++;
                }
                
                auto& result = results[idx];
                result.started.store(true);
                runBuildProcess(m_configs[idx], result);

                result.finished.store(true);

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    activeCount--;
                    completedCount++;
                    if (result.exitCode == 0) successCount++;
                    else failCount++;

                    printProgress(completedCount, total, successCount, failCount,
                                  activeCount, result.label, result.exitCode);
                }
                cv.notify_all();
            }
        };

        int numWorkers = std::min(m_maxParallel, static_cast<int>(total));
        std::vector<std::thread> workers;
        for (int i = 0; i < numWorkers; i++) {
            workers.emplace_back(worker);
        }
        for (auto& w : workers) { if (w.joinable()) w.join(); }

        auto endTime = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(endTime - startTime).count();

        printSummary(elapsed, total, successCount, failCount, results);
        writeLog(results);
    }

private:
    int m_maxParallel;
    int m_jobsPerTask;
    std::vector<BuildConfig> m_configs;
    std::atomic<size_t> m_nextIndex{0};

    std::string formatLabel(const BuildConfig& cfg) const {
        return cfg.toolchain + "/" + cfg.crt + "/" + cfg.buildType + "/-" + cfg.optimization;
    }

    void printHeader(size_t total) const {
        printf("\n");
        printf("======================================================================\n");
        printf("  libzplay Lite C++ Parallel Builder v2.0\n");
        printf("======================================================================\n");
        
        auto now = std::chrono::system_clock::now();
        time_t tt = std::chrono::system_clock::to_time_t(now);
        struct tm tm_buf;
        localtime_s(&tm_buf, &tt);
        printf("  Date: %04d-%02d-%02d %02d:%02d:%02d\n",
               tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
               tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec);
        printf("  Total configs: %zu | Parallel: %d x %d threads\n", 
               total, m_maxParallel, m_jobsPerTask);
        printf("======================================================================\n\n");
    }

    void runBuildProcess(const BuildConfig& cfg, BuildResult& result) {
        std::string projectRoot = getProjectRoot();
        
        // Split toolchain into compiler + arch for build.ps1
        // e.g. "msvc-x64" -> compiler="msvc" arch="x64"
        std::string compiler = cfg.toolchain;
        std::string arch = "x64";
        
        size_t dashPos = cfg.toolchain.rfind('-');
        if (dashPos != std::string::npos) {
            std::string maybeArch = cfg.toolchain.substr(dashPos + 1);
            if (maybeArch == "x64" || maybeArch == "x86" || maybeArch == "arm64") {
                compiler = cfg.toolchain.substr(0, dashPos);
                arch = maybeArch;
            }
        }
        
        // Map llvm-22 -> llvm-22 (keep as-is), vs-clang-arm64 -> vs-clang with arch=arm64
        std::string cmd = "powershell.exe -NoProfile -Command \"" +
            std::string("\"") + projectRoot + "\\build.ps1\" " +
            "-Compiler " + compiler + " " +
            "-Arch " + arch + " " +
            "-CRT " + cfg.crt + " " +
            "-BuildType " + cfg.buildType + " " +
            "-Optimization " + cfg.optimization + " " +
            "-Jobs " + std::to_string(m_jobsPerTask) + "\"";

        SECURITY_ATTRIBUTES sa = {};
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;

        HANDLE hReadOut, hWriteOut;
        CreatePipe(&hReadOut, &hWriteOut, &sa, 0);
        SetHandleInformation(hReadOut, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.hStdOutput = hWriteOut;
        si.hStdError = hWriteOut;
        si.dwFlags = STARTF_USESTDHANDLES;

        PROCESS_INFORMATION pi = {};

        char* cmdCopy = _strdup(cmd.c_str());
        BOOL ok = CreateProcessA(NULL, cmdCopy, NULL, NULL, TRUE,
                                  CREATE_NO_WINDOW, NULL, projectRoot.c_str(), &si, &pi);
        free(cmdCopy);
        CloseHandle(hWriteOut);

        if (!ok) {
            result.exitCode = GetLastError();
            result.output = "CreateProcess failed: " + std::to_string(result.exitCode);
            CloseHandle(hReadOut);
            return;
        }

        // Read output in thread
        std::string outBuf;
        std::thread reader([&]() {
            char buf[4096];
            DWORD nRead;
            while (ReadFile(hReadOut, buf, sizeof(buf)-1, &nRead, NULL) && nRead > 0) {
                buf[nRead] = 0;
                outBuf += buf;
            }
        });
        reader.join();

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        result.exitCode = (int)exitCode;
        result.output = outBuf;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hReadOut);
    }

    void printProgress(int completed, int total, int ok, int failed,
                       int active, const std::string& label, int exitCode) const {
        double pct = total > 0 ? (double)completed / total * 100.0 : 0;
        
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD color = FOREGROUND_GREEN;
        const char* status = "OK";
        if (exitCode != 0) { color = FOREGROUND_RED; status = "FAIL"; }
        
        SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_CYAN);
        printf("[%3d/%-3zu]", completed, total);
        
        SetConsoleTextAttribute(hConsole, color);
        printf(" %-45s -> %-4s", label.c_str(), status);
        
        SetConsoleTextAttribute(hConsole, 8); // dark gray
        printf(" | Active: %-2d | OK: %-3d | Fail: %-3d (%5.1f%%)\n",
               active, ok, failed, pct);
        
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    void printSummary(double elapsed, size_t total, int ok, int failed,
                      const std::vector<BuildResult>& results) const {
        printf("\n======================================================================\n");
        printf("  BUILD COMPLETE\n");
        printf("======================================================================\n");
        printf("  Elapsed: %.1fs (%.2fmin)\n", elapsed, elapsed/60.0);
        printf("  Total: %-4zu | OK: %-4d | Fail: %-4d\n", total, ok, failed);
        if (total > 0) printf("  Rate: %.1f%% | Throughput: ~%.1f configs/min\n",
            (double)(ok)/total*100.0, total/std::max(elapsed/60.0, 0.01));
        printf("======================================================================\n");

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (failed > 0) {
            printf("\n  FAILED configurations:\n");
            int shown = 0;
            for (const auto& r : results) {
                if (r.exitCode != 0 && shown < 30) {
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                    printf("    [FAIL] %s (code: %d)\n", r.label.c_str(), r.exitCode.load());
                    
                    // Show error context
                    size_t errPos = r.output.find("error ");
                    size_t failPos = r.output.find("FAILED");
                    size_t pos = r.output.npos;
                    if (errPos != r.output.npos) pos = errPos;
                    if (failPos != r.output.npos && failPos < pos) pos = failPos;
                    
                    if (pos != r.output.npos && pos < r.output.size() - 50) {
                        std::string snippet = r.output.substr(pos, std::min((size_t)120, r.output.size() - pos));
                        // Replace newlines with spaces
                        for (char& c : snippet) { if (c == '\n' || c == '\r') c = ' '; }
                        printf("           => %.120s\n", snippet.c_str());
                    }
                    shown++;
                }
            }
            if (failed > 30) printf("    ... and %d more (see log)\n", failed - 30);
            
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        } else {
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            printf("  ALL BUILDS SUCCESSFUL!\n");
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
    }

    void writeLog(const std::vector<BuildResult>& results) const {
        std::ofstream log(getProjectRoot() + "\\build_log_detailed.txt");
        if (!log.is_open()) return;
        
        log << "libzplay Detailed Build Log - " << results.size() << " configs\n";
        log << "================================================\n\n";
        
        for (const auto& r : results) {
            log << "[" << (r.exitCode==0 ? "OK" : "FAIL") << "] " << r.label << "\n";
            log << "   Exit Code: " << r.exitCode.load() << "\n";
            
            // Write last 100 lines of output
            std::istringstream stream(r.output);
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(stream, line)) lines.push_back(line);
            
            size_t start = lines.size() > 80 ? lines.size() - 80 : 0;
            for (size_t i = start; i < lines.size(); i++) {
                log << "   " << lines[i] << "\n";
            }
            log << "\n";
        }
        log.close();
        printf("  Log: build_log_detailed.txt\n");
    }

    std::string getProjectRoot() const {
        char buf[MAX_PATH];
        GetModuleFileNameA(NULL, buf, MAX_PATH);
        std::string path(buf);
        size_t pos = path.find_last_of("\\/");
        return (pos != std::string::npos) ? path.substr(0, pos) : ".";
    }
};

int main(int argc, char* argv[]) {
    int maxParallel = 2;
    int jobsPerTask = 8;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-p" || arg == "--parallel") && i+1 < argc) maxParallel = atoi(argv[++i]);
        if ((arg == "-j" || arg == "--jobs") && i+1 < argc) jobsPerTask = atoi(argv[++i]);
        if (arg == "-h" || arg == "--help") {
            printf("Usage: builder [-p N] [-j N]\n  -p N  parallel builds (default 2)\n  -j N  threads per build (default 8)\n");
            return 0;
        }
    }

    ParallelBuilder builder(maxParallel, jobsPerTask);

    const char* toolchains[] = {"msvc-x64","msvc-x86","msvc-arm64","mingw-x64","mingw-x86","llvm","llvm-22","vs-clang-x64","vs-clang-arm64"};
    const char* crts[] = {"MSVCRT","UCRT"};
    const char* types[] = {"release","debug"};
    const char* opts[] = {"O0","O3","Os","OF"};

    for (auto tc : toolchains)
        for (auto crt : crts) {
            std::string t(tc);
            if (t.find("mingw-x86") != std::string::npos && std::string(crt) == "UCRT") continue;
            for (auto bt : types)
                for (auto opt : opts)
                    builder.addConfig({tc,crt,bt,opt});
        }

    builder.runAll();
    return 0;
}
