#include <windows.h>
#include <dxgi.h>

// ============================================================================
// DXGI Proxy DLL Export Stubs
// Allows the cheat to be loaded automatically as dxgi.dll without an injector
// ============================================================================

static HMODULE g_hRealDXGI = nullptr;

typedef HRESULT (WINAPI* tCreateDXGIFactory)(REFIID riid, void **ppFactory);
typedef HRESULT (WINAPI* tCreateDXGIFactory1)(REFIID riid, void **ppFactory);
typedef HRESULT (WINAPI* tCreateDXGIFactory2)(UINT Flags, REFIID riid, void **ppFactory);
typedef HRESULT (WINAPI* tDXGIGetDebugInterface1)(UINT Flags, REFIID riid, void **pDebug);

static tCreateDXGIFactory oCreateDXGIFactory = nullptr;
static tCreateDXGIFactory1 oCreateDXGIFactory1 = nullptr;
static tCreateDXGIFactory2 oCreateDXGIFactory2 = nullptr;
static tDXGIGetDebugInterface1 oDXGIGetDebugInterface1 = nullptr;

void LoadRealDXGI() {
    if (g_hRealDXGI) return;

    char sysDir[MAX_PATH];
    GetSystemDirectoryA(sysDir, MAX_PATH);
    char realPath[MAX_PATH];
    wsprintfA(realPath, "%s\\dxgi.dll", sysDir);

    g_hRealDXGI = LoadLibraryA(realPath);
    if (g_hRealDXGI) {
        oCreateDXGIFactory = (tCreateDXGIFactory)GetProcAddress(g_hRealDXGI, "CreateDXGIFactory");
        oCreateDXGIFactory1 = (tCreateDXGIFactory1)GetProcAddress(g_hRealDXGI, "CreateDXGIFactory1");
        oCreateDXGIFactory2 = (tCreateDXGIFactory2)GetProcAddress(g_hRealDXGI, "CreateDXGIFactory2");
        oDXGIGetDebugInterface1 = (tDXGIGetDebugInterface1)GetProcAddress(g_hRealDXGI, "DXGIGetDebugInterface1");
    }
}

extern "C" {

__declspec(dllexport) HRESULT WINAPI CreateDXGIFactory(REFIID riid, void **ppFactory) {
    LoadRealDXGI();
    if (oCreateDXGIFactory)
        return oCreateDXGIFactory(riid, ppFactory);
    return E_FAIL;
}

__declspec(dllexport) HRESULT WINAPI CreateDXGIFactory1(REFIID riid, void **ppFactory) {
    LoadRealDXGI();
    if (oCreateDXGIFactory1)
        return oCreateDXGIFactory1(riid, ppFactory);
    return E_FAIL;
}

__declspec(dllexport) HRESULT WINAPI CreateDXGIFactory2(UINT Flags, REFIID riid, void **ppFactory) {
    LoadRealDXGI();
    if (oCreateDXGIFactory2)
        return oCreateDXGIFactory2(Flags, riid, ppFactory);
    return E_FAIL;
}

__declspec(dllexport) HRESULT WINAPI DXGIGetDebugInterface1(UINT Flags, REFIID riid, void **pDebug) {
    LoadRealDXGI();
    if (oDXGIGetDebugInterface1)
        return oDXGIGetDebugInterface1(Flags, riid, pDebug);
    return E_FAIL;
}

}
