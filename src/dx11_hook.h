#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

extern bool g_KeysDown[256];

namespace DX11Hook {
    bool Initialize();
    void Uninitialize();
}
