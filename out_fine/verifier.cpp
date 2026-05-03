// verifier.cpp - Output directory verification tool
// Compile: g++ -std=c++17 -O2 verifier.cpp -o verifier.exe

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>

struct ConfigResult {
    std::string path;
    bool hasBin = false;
    bool hasLib = false;
    bool hasObj = false;
    bool hasDbg = false;
    bool hasInclude = false;
    bool hasExample = false;
    int dllCount = 0;
    int libCount = 0;
    int objCount = 0;
    int dbgCount = 0;
    int headerCount = 0;
    int exampleCount = 0;
};

std::vector<std::string> findFiles(const std::string& dir, const std::string& pattern, bool recursive) {
    std::vector<std::string> files;
    WIN32_FIND_DATAA fd;
    std::string searchPath = dir + "\\" + pattern;
    
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(fd.cFileName);
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
    
    if (recursive) {
        std::string subSearch = dir + "\\*";
        hFind = FindFirstFileA(subSearch.c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
                    fd.cFileName[0] != '.' && 
                    strcmp(fd.cFileName, "..") != 0) {
                    std::string subDir = dir + "\\" + fd.cFileName;
                    auto subFiles = findFiles(subDir, pattern, true);
                    files.insert(files.end(), subFiles.begin(), subFiles.end());
                }
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
    }
    
    return files;
}

int countFiles(const std::string& dir, const std::vector<std::string>& extensions, bool recursive) {
    int count = 0;
    for (const auto& ext : extensions) {
        count += (int)findFiles(dir, ext, recursive).size();
    }
    return count;
}

bool dirExists(const std::string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

ConfigResult verifyConfig(const std::string& configPath) {
    ConfigResult result;
    result.path = configPath;
    
    std::string binDir = configPath + "\\bin";
    std::string libDir = configPath + "\\lib";
    std::string objDir = configPath + "\\obj";
    std::string dbgDir = configPath + "\\dbg";
    std::string incDir = configPath + "\\include";
    std::string exDir = configPath + "\\example";
    
    if (dirExists(binDir)) {
        result.hasBin = true;
        result.dllCount = countFiles(binDir, {"*.dll"}, false);
    }
    
    if (dirExists(libDir)) {
        result.hasLib = true;
        result.libCount = countFiles(libDir, {"*.lib", "*.a"}, false);
    }
    
    if (dirExists(objDir)) {
        result.hasObj = true;
        result.objCount = countFiles(objDir, {"*.obj", "*.o"}, true);
    }
    
    if (dirExists(dbgDir)) {
        result.hasDbg = true;
        result.dbgCount = countFiles(dbgDir, {"*"}, false);
    }
    
    if (dirExists(incDir)) {
        result.hasInclude = true;
        result.headerCount = countFiles(incDir, {"*.h"}, false);
    }
    
    if (dirExists(exDir)) {
        result.hasExample = true;
        result.exampleCount = countFiles(exDir, {"*"}, false);
    }
    
    return result;
}

std::vector<std::string> findConfigDirs(const std::string& baseDir) {
    std::vector<std::string> configs;
    WIN32_FIND_DATAA fd1, fd2, fd3, fd4, fd5;
    
    std::string s1 = baseDir + "\\*";
    HANDLE h1 = FindFirstFileA(s1.c_str(), &fd1);
    if (h1 == INVALID_HANDLE_VALUE) return configs;
    
    do {
        if (!(fd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd1.cFileName[0] == '.') continue;
        std::string p1 = baseDir + "\\" + fd1.cFileName;
        
        std::string s2 = p1 + "\\*";
        HANDLE h2 = FindFirstFileA(s2.c_str(), &fd2);
        if (h2 == INVALID_HANDLE_VALUE) continue;
        
        do {
            if (!(fd2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd2.cFileName[0] == '.') continue;
            std::string p2 = p1 + "\\" + fd2.cFileName;
            
            std::string s3 = p2 + "\\*";
            HANDLE h3 = FindFirstFileA(s3.c_str(), &fd3);
            if (h3 == INVALID_HANDLE_VALUE) continue;
            
            do {
                if (!(fd3.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd3.cFileName[0] == '.') continue;
                std::string p3 = p2 + "\\" + fd3.cFileName;
                
                std::string s4 = p3 + "\\*";
                HANDLE h4 = FindFirstFileA(s4.c_str(), &fd4);
                if (h4 == INVALID_HANDLE_VALUE) continue;
                
                do {
                    if (!(fd4.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd4.cFileName[0] == '.') continue;
                    std::string p4 = p3 + "\\" + fd4.cFileName;
                    
                    std::string s5 = p4 + "\\*";
                    HANDLE h5 = FindFirstFileA(s5.c_str(), &fd5);
                    if (h5 == INVALID_HANDLE_VALUE) continue;
                    
                    do {
                        if (!(fd5.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd5.cFileName[0] == '.') continue;
                        std::string opt = fd5.cFileName;
                        if (opt.size() >= 2 && opt[0] == '-' && (opt[1] == 'O' || opt[1] == 'o')) {
                            configs.push_back(p4 + "\\" + opt);
                        }
                    } while (FindNextFileA(h5, &fd5));
                    FindClose(h5);
                    
                } while (FindNextFileA(h4, &fd4));
                FindClose(h4);
                
            } while (FindNextFileA(h3, &fd3));
            FindClose(h3);
            
        } while (FindNextFileA(h2, &fd2));
        FindClose(h2);
        
    } while (FindNextFileA(h1, &fd1));
    FindClose(h1);
    
    return configs;
}

int main(int argc, char* argv[]) {
    printf("\n");
    printf("======================================================================\n");
    printf("  libzplay Output Directory Verifier\n");
    printf("======================================================================\n\n");
    
    std::string baseDir = "out";
    if (argc > 1) baseDir = argv[1];
    
    auto startTime = std::chrono::steady_clock::now();
    
    printf("  Scanning: %s\n\n", baseDir.c_str());
    
    std::vector<std::string> configs = findConfigDirs(baseDir);
    printf("  Found: %zu configurations\n\n", configs.size());
    
    int complete = 0;
    int incomplete = 0;
    std::vector<ConfigResult> issues;
    
    for (size_t i = 0; i < configs.size(); i++) {
        ConfigResult r = verifyConfig(configs[i]);
        
        bool ok = r.hasBin && r.hasLib && r.hasObj && r.hasDbg && r.hasInclude && r.hasExample;
        ok = ok && r.dllCount >= 1 && r.libCount >= 10 && r.objCount >= 100 && r.headerCount >= 3;
        
        size_t pos = r.path.find("out\\");
        std::string shortPath = (pos != std::string::npos) ? r.path.substr(pos + 4) : r.path;
        
        if (ok) {
            complete++;
            printf("\r  [%3zu/%-3zu] OK        - %s", i+1, configs.size(), shortPath.c_str());
        } else {
            incomplete++;
            issues.push_back(r);
            printf("\r  [%3zu/%-3zu] INCOMPLETE - %s\n", i+1, configs.size(), shortPath.c_str());
        }
    }
    
    auto endTime = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(endTime - startTime).count();
    
    printf("\n\n");
    printf("======================================================================\n");
    printf("  VERIFICATION RESULTS\n");
    printf("======================================================================\n");
    printf("  Total:     %zu\n", configs.size());
    printf("  Complete:  %d\n", complete);
    printf("  Incomplete: %d\n", incomplete);
    printf("  Time:      %.2fs\n", elapsed);
    printf("======================================================================\n");
    
    if (!issues.empty()) {
        printf("\n  INCOMPLETE CONFIGURATIONS:\n\n");
        for (const auto& r : issues) {
            size_t pos = r.path.find("out\\");
            std::string shortPath = (pos != std::string::npos) ? r.path.substr(pos + 4) : r.path;
            printf("  %s\n", shortPath.c_str());
            printf("    bin/    %-3s (%d DLLs)    ", r.hasBin ? "OK" : "MISS", r.dllCount);
            printf("lib/     %-3s (%d libs)\n", r.hasLib ? "OK" : "MISS", r.libCount);
            printf("    obj/    %-3s (%d files)   ", r.hasObj ? "OK" : "MISS", r.objCount);
            printf("dbg/     %-3s (%d files)\n", r.hasDbg ? "OK" : "MISS", r.dbgCount);
            printf("    include/%-3s (%d headers) ", r.hasInclude ? "OK" : "MISS", r.headerCount);
            printf("example/ %-3s (%d files)\n", r.hasExample ? "OK" : "MISS", r.exampleCount);
            printf("\n");
        }
    } else {
        printf("\n  ALL CONFIGURATIONS COMPLETE!\n");
    }
    
    return incomplete > 0 ? 1 : 0;
}
