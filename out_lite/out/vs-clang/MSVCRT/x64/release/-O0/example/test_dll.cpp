#include <windows.h>
#include <stdio.h>
#include "libzplay_dll.h"

int main(int argc, char* argv[])
{
    printf("libzplay DLL Test (Lite)\n");
    printf("========================\n\n");
    
    libZPlay::ZPlay *player = libZPlay::CreateZPlay();
    if (!player) {
        printf("ERROR: Failed to create ZPlay instance\n");
        return 1;
    }
    
    printf("ZPlay instance created successfully!\n");
    printf("Version: %u\n", player->GetVersion());
    
    const char* testFile = (argc > 1) ? argv[1] : "test.mp3";
    
    int result = player->OpenFile(testFile, libZPlay::sfAutodetect);
    if (result == 0) {
        printf("Note: Could not open test file: %s\n", testFile);
    } else {
        printf("File opened: %s\n", testFile);
        printf("Press Enter to play...\n");
        getchar();
        player->Play();
        printf("Playing... Press Enter to stop.\n");
        getchar();
        player->Stop();
    }
    
    player->Release();
    printf("\nDLL test completed successfully!\n");
    return 0;
}
