#include <windows.h>
#include <xinput.h>

// ============================================================================
// XINPUT1_3 Proxy DLL Export Stubs
// Highly recommended for Steam Proton / Wine on Linux
// Avoids all DXVK conflicts by proxying the input library instead of dxgi
// ============================================================================

static HMODULE g_hRealXInput = nullptr;

typedef DWORD (WINAPI* tXInputGetState)(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD (WINAPI* tXInputSetState)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
typedef DWORD (WINAPI* tXInputGetCapabilities)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);
typedef DWORD (WINAPI* tXInputGetBatteryInformation)(DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION* pBatteryInformation);

static tXInputGetState oXInputGetState = nullptr;
static tXInputSetState oXInputSetState = nullptr;
static tXInputGetCapabilities oXInputGetCapabilities = nullptr;
static tXInputGetBatteryInformation oXInputGetBatteryInformation = nullptr;

static void LoadRealXInput() {
    if (g_hRealXInput) return;

    // Load xinput1_4.dll from system32 (always available in Wine/Proton and Windows 10/11)
    // Using xinput1_4 avoids any circular loading loops when xinput1_3 is overridden to native
    char sysDir[MAX_PATH];
    GetSystemDirectoryA(sysDir, MAX_PATH);

    char path14[MAX_PATH];
    wsprintfA(path14, "%s\\xinput1_4.dll", sysDir);
    g_hRealXInput = LoadLibraryA(path14);

    if (!g_hRealXInput) {
        char path91[MAX_PATH];
        wsprintfA(path91, "%s\\xinput9_1_0.dll", sysDir);
        g_hRealXInput = LoadLibraryA(path91);
    }

    if (g_hRealXInput) {
        oXInputGetState = (tXInputGetState)GetProcAddress(g_hRealXInput, "XInputGetState");
        oXInputSetState = (tXInputSetState)GetProcAddress(g_hRealXInput, "XInputSetState");
        oXInputGetCapabilities = (tXInputGetCapabilities)GetProcAddress(g_hRealXInput, "XInputGetCapabilities");
        oXInputGetBatteryInformation = (tXInputGetBatteryInformation)GetProcAddress(g_hRealXInput, "XInputGetBatteryInformation");
    }
}

extern "C" {

__declspec(dllexport) DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
    LoadRealXInput();
    if (oXInputGetState)
        return oXInputGetState(dwUserIndex, pState);
    return ERROR_DEVICE_NOT_CONNECTED;
}

__declspec(dllexport) DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
    LoadRealXInput();
    if (oXInputSetState)
        return oXInputSetState(dwUserIndex, pVibration);
    return ERROR_DEVICE_NOT_CONNECTED;
}

__declspec(dllexport) DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities) {
    LoadRealXInput();
    if (oXInputGetCapabilities)
        return oXInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);
    return ERROR_DEVICE_NOT_CONNECTED;
}

__declspec(dllexport) DWORD WINAPI XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION* pBatteryInformation) {
    LoadRealXInput();
    if (oXInputGetBatteryInformation)
        return oXInputGetBatteryInformation(dwUserIndex, devType, pBatteryInformation);
    return ERROR_DEVICE_NOT_CONNECTED;
}

}
