#include <windows.h>
#include "dx11_hook.h"

DWORD WINAPI MainThread(LPVOID lpParam) {
    // Wait briefly for UE4 engine window & DirectX 11 to spin up
    Sleep(1500);

    DX11Hook::Initialize();

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, 0, nullptr);
        if (hThread) {
            CloseHandle(hThread);
        }
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        DX11Hook::Uninitialize();
    }
    return TRUE;
}
