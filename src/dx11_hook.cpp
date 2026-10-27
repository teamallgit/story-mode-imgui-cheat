#include "dx11_hook.h"
#include "menu.h"
#include "../minhook/include/MinHook.h"
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../imgui/backends/imgui_impl_dx11.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool g_KeysDown[256] = { false };

namespace DX11Hook {

typedef HRESULT (WINAPI* tPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT (WINAPI* tResizeBuffers)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

static tPresent oPresent = nullptr;
static tResizeBuffers oResizeBuffers = nullptr;
static WNDPROC oWndProc = nullptr;

static HWND g_hWnd = nullptr;
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static bool g_bInitialized = false;

static void CreateRenderTarget(IDXGISwapChain* pSwapChain) {
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer))) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

static void CleanupRenderTarget() {
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

static LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_INPUT) {
        RAWINPUT raw;
        UINT size = sizeof(raw);
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1) {
            if (raw.header.dwType == RIM_TYPEKEYBOARD) {
                USHORT vkey = raw.data.keyboard.VKey;
                USHORT makeCode = raw.data.keyboard.MakeCode;
                USHORT flags = raw.data.keyboard.Flags;

                if (vkey == 0 || vkey == 0xFF) {
                    vkey = (USHORT)MapVirtualKeyA(makeCode, MAPVK_VSC_TO_VK_EX);
                }
                if (vkey < 256) {
                    if (flags & RI_KEY_BREAK) {
                        g_KeysDown[vkey] = false;
                    } else {
                        g_KeysDown[vkey] = true;
                        if (vkey == VK_END || vkey == VK_INSERT) {
                            g_Settings.bMenuOpen = !g_Settings.bMenuOpen;
                        }
                    }
                }
            }
        }
    } else if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
        if (wParam < 256) g_KeysDown[wParam] = true;
    } else if (msg == WM_KEYUP || msg == WM_SYSKEYUP) {
        if (wParam < 256) g_KeysDown[wParam] = false;
    }

    if (msg == WM_KEYDOWN && (wParam == VK_END || wParam == VK_INSERT || wParam == VK_DELETE)) {
        g_Settings.bMenuOpen = !g_Settings.bMenuOpen;
        return 0;
    }

    if (g_Settings.bMenuOpen) {
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        // Block game mouse and keyboard input while interacting with menu
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse && (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST))
            return 1;
        if (io.WantCaptureKeyboard && (msg >= WM_KEYFIRST && msg <= WM_KEYLAST))
            return 1;
    } else {
        // Forward input to ImGui even when menu is closed so key states stay fresh, without blocking game
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    }

    return CallWindowProcA(oWndProc, hWnd, msg, wParam, lParam);
}

static HRESULT WINAPI HookedPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_bInitialized) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice))) {
            g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);

            DXGI_SWAP_CHAIN_DESC desc;
            pSwapChain->GetDesc(&desc);
            g_hWnd = desc.OutputWindow;

            CreateRenderTarget(pSwapChain);

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            CheatMenu::InitializeTheme();

            ImGui_ImplWin32_Init(g_hWnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

            oWndProc = (WNDPROC)SetWindowLongPtrA(g_hWnd, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);

            g_bInitialized = true;
        }
    }

    if (g_bInitialized && g_mainRenderTargetView) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Run cheat tick (update noclip, godmode, speed, etc.)
        CheatMenu::UpdateCheats();

        ImGuiIO& io = ImGui::GetIO();
        float screenWidth = io.DisplaySize.x;
        float screenHeight = io.DisplaySize.y;

        // Render ESP overlay
        CheatMenu::RenderESP(ImGui::GetBackgroundDrawList(), screenWidth, screenHeight);

        // Render cheat menu
        CheatMenu::Render();

        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI HookedResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    CleanupRenderTarget();
    HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    CreateRenderTarget(pSwapChain);
    return hr;
}

bool Initialize() {
    if (MH_Initialize() != MH_OK)
        return false;

    // Create a dummy DirectX 11 device and swapchain to retrieve VMT pointers
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, DefWindowProcA, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX11_DUMMY", NULL };
    RegisterClassExA(&wc);
    HWND hWnd = CreateWindowA("DX11_DUMMY", "", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, wc.hInstance, NULL);

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };

    DXGI_SWAP_CHAIN_DESC scd = { 0 };
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* pDummySwapChain = nullptr;
    ID3D11Device* pDummyDevice = nullptr;
    ID3D11DeviceContext* pDummyContext = nullptr;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 2, D3D11_SDK_VERSION, &scd, &pDummySwapChain, &pDummyDevice, &featureLevel, &pDummyContext);

    if (FAILED(hr)) {
        // Fallback to WARP driver (useful on Wine/Proton setups)
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, featureLevels, 2, D3D11_SDK_VERSION, &scd, &pDummySwapChain, &pDummyDevice, &featureLevel, &pDummyContext);
    }

    if (SUCCEEDED(hr) && pDummySwapChain) {
        void** pVMT = *(void***)pDummySwapChain;
        void* pPresentTarget = pVMT[8];
        void* pResizeBuffersTarget = pVMT[13];

        MH_CreateHook(pPresentTarget, (void*)HookedPresent, (void**)&oPresent);
        MH_EnableHook(pPresentTarget);

        MH_CreateHook(pResizeBuffersTarget, (void*)HookedResizeBuffers, (void**)&oResizeBuffers);
        MH_EnableHook(pResizeBuffersTarget);

        pDummySwapChain->Release();
        pDummyDevice->Release();
        pDummyContext->Release();
    }

    DestroyWindow(hWnd);
    UnregisterClassA("DX11_DUMMY", wc.hInstance);

    return (oPresent != nullptr);
}

void Uninitialize() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if (oWndProc && g_hWnd) {
        SetWindowLongPtrA(g_hWnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    }

    CleanupRenderTarget();

    if (g_bInitialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_bInitialized = false;
    }
}

} // namespace DX11Hook
