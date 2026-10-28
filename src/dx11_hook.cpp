#include "dx11_hook.h"
#include "menu.h"
#include "ambient_shaders.h"
#include "../minhook/include/MinHook.h"
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../imgui/backends/imgui_impl_dx11.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool g_KeysDown[256] = { false };

namespace DX11Hook {

typedef HRESULT (WINAPI* tPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT (WINAPI* tResizeBuffers)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
typedef WNDPROC tWndProc;

static tPresent oPresent = nullptr;
static tResizeBuffers oResizeBuffers = nullptr;
static WNDPROC oWndProc = nullptr;

static HWND g_hWnd = nullptr;
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static bool g_bInitialized = false;

// ============================================================================
// Native DirectX 11 Ambient Lighting & Color Shader Resources
// ============================================================================
struct AmbientCB {
    float Brightness;
    float TintR;
    float TintG;
    float TintB;
    float EnableTint;
    float Padding[3];
};

static ID3D11VertexShader* g_pAmbientVS = nullptr;
static ID3D11PixelShader* g_pAmbientPS = nullptr;
static ID3D11Buffer* g_pAmbientCB = nullptr;
static ID3D11SamplerState* g_pAmbientSampler = nullptr;
static ID3D11Texture2D* g_pCopyTexture = nullptr;
static ID3D11ShaderResourceView* g_pCopySRV = nullptr;
static UINT g_CopyWidth = 0;
static UINT g_CopyHeight = 0;
static bool g_bAmbientInitialized = false;

static void ReleaseAmbientCopyResources() {
    if (g_pCopySRV) {
        g_pCopySRV->Release();
        g_pCopySRV = nullptr;
    }
    if (g_pCopyTexture) {
        g_pCopyTexture->Release();
        g_pCopyTexture = nullptr;
    }
    g_CopyWidth = 0;
    g_CopyHeight = 0;
}

static void InitAmbientLighting(ID3D11Device* pDevice) {
    if (g_bAmbientInitialized || !pDevice) return;

    HRESULT hrVS = pDevice->CreateVertexShader(g_AmbientVSBytecode, sizeof(g_AmbientVSBytecode), nullptr, &g_pAmbientVS);
    HRESULT hrPS = pDevice->CreatePixelShader(g_AmbientPSBytecode, sizeof(g_AmbientPSBytecode), nullptr, &g_pAmbientPS);

    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory(&cbDesc, sizeof(cbDesc));
    cbDesc.ByteWidth = sizeof(AmbientCB);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HRESULT hrCB = pDevice->CreateBuffer(&cbDesc, nullptr, &g_pAmbientCB);

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HRESULT hrSamp = pDevice->CreateSamplerState(&sampDesc, &g_pAmbientSampler);

    if (SUCCEEDED(hrVS) && SUCCEEDED(hrPS) && SUCCEEDED(hrCB) && SUCCEEDED(hrSamp)) {
        g_bAmbientInitialized = true;
    }
}

static void CleanupAmbientLighting() {
    ReleaseAmbientCopyResources();
    if (g_pAmbientVS) { g_pAmbientVS->Release(); g_pAmbientVS = nullptr; }
    if (g_pAmbientPS) { g_pAmbientPS->Release(); g_pAmbientPS = nullptr; }
    if (g_pAmbientCB) { g_pAmbientCB->Release(); g_pAmbientCB = nullptr; }
    if (g_pAmbientSampler) { g_pAmbientSampler->Release(); g_pAmbientSampler = nullptr; }
    g_bAmbientInitialized = false;
}

static void RenderAmbientLightingPass(IDXGISwapChain* pSwapChain) {
    if (!g_Settings.bAmbientLighting || !g_bAmbientInitialized || !g_pd3dDeviceContext || !g_mainRenderTargetView)
        return;

    ID3D11Texture2D* pBackBuffer = nullptr;
    if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer)) || !pBackBuffer)
        return;

    D3D11_TEXTURE2D_DESC bbDesc;
    pBackBuffer->GetDesc(&bbDesc);

    // Recreate copy texture if dimensions changed
    if (!g_pCopyTexture || g_CopyWidth != bbDesc.Width || g_CopyHeight != bbDesc.Height) {
        ReleaseAmbientCopyResources();

        D3D11_TEXTURE2D_DESC copyDesc = bbDesc;
        copyDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        copyDesc.Usage = D3D11_USAGE_DEFAULT;
        copyDesc.CPUAccessFlags = 0;
        copyDesc.MiscFlags = 0;

        if (SUCCEEDED(g_pd3dDevice->CreateTexture2D(&copyDesc, nullptr, &g_pCopyTexture))) {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            ZeroMemory(&srvDesc, sizeof(srvDesc));
            DXGI_FORMAT srvFormat = copyDesc.Format;
            if (srvFormat == DXGI_FORMAT_R8G8B8A8_TYPELESS) srvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            else if (srvFormat == DXGI_FORMAT_B8G8R8A8_TYPELESS) srvFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
            srvDesc.Format = srvFormat;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = 1;

            if (SUCCEEDED(g_pd3dDevice->CreateShaderResourceView(g_pCopyTexture, &srvDesc, &g_pCopySRV))) {
                g_CopyWidth = bbDesc.Width;
                g_CopyHeight = bbDesc.Height;
            }
        }
    }

    if (!g_pCopyTexture || !g_pCopySRV) {
        pBackBuffer->Release();
        return;
    }

    // Hardware GPU-to-GPU copy from backbuffer into our sampled texture
    g_pd3dDeviceContext->CopyResource(g_pCopyTexture, pBackBuffer);
    pBackBuffer->Release();

    // Update constant buffer parameters
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(g_pd3dDeviceContext->Map(g_pAmbientCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        AmbientCB* cb = (AmbientCB*)mapped.pData;
        cb->Brightness = g_Settings.AmbientBrightness;
        cb->EnableTint = g_Settings.bAmbientColor ? 1.0f : 0.0f;

        float r = 1.0f, g = 1.0f, b = 1.0f;
        if (g_Settings.bAmbientColor) {
            switch (g_Settings.AmbientColorMode) {
                case 0: r = 1.0f; g = 1.0f; b = 1.0f; break; // Natural White
                case 1: r = 1.0f; g = 0.05f; b = 0.05f; break; // Blood Red
                case 2: r = 0.08f; g = 1.0f; b = 0.15f; break; // Toxic Green
                case 3: r = 0.12f; g = 0.35f; b = 1.0f; break; // Deep Blue
                case 4: r = 0.95f; g = 0.15f; b = 1.0f; break; // Neon Purple
                case 5: // Custom
                    r = g_Settings.CustomAmbientColor[0];
                    g = g_Settings.CustomAmbientColor[1];
                    b = g_Settings.CustomAmbientColor[2];
                    break;
                default: break;
            }
        }
        cb->TintR = r;
        cb->TintG = g;
        cb->TintB = b;
        cb->Padding[0] = 0.0f; cb->Padding[1] = 0.0f; cb->Padding[2] = 0.0f;

        g_pd3dDeviceContext->Unmap(g_pAmbientCB, 0);
    }

    // Bind backbuffer as render target
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);

    // Setup viewport
    D3D11_VIEWPORT vp;
    vp.Width = (float)bbDesc.Width;
    vp.Height = (float)bbDesc.Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    g_pd3dDeviceContext->RSSetViewports(1, &vp);

    // Set state for fullscreen triangle pass
    g_pd3dDeviceContext->OMSetDepthStencilState(nullptr, 0);
    g_pd3dDeviceContext->RSSetState(nullptr);
    g_pd3dDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    g_pd3dDeviceContext->IASetInputLayout(nullptr);
    g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    g_pd3dDeviceContext->VSSetShader(g_pAmbientVS, nullptr, 0);
    g_pd3dDeviceContext->PSSetShader(g_pAmbientPS, nullptr, 0);
    g_pd3dDeviceContext->PSSetConstantBuffers(0, 1, &g_pAmbientCB);
    g_pd3dDeviceContext->PSSetShaderResources(0, 1, &g_pCopySRV);
    g_pd3dDeviceContext->PSSetSamplers(0, 1, &g_pAmbientSampler);

    // Draw fullscreen triangle (3 vertices)
    g_pd3dDeviceContext->Draw(3, 0);

    // Unbind shader resource to avoid pipeline hazards
    ID3D11ShaderResourceView* nullSRV = nullptr;
    g_pd3dDeviceContext->PSSetShaderResources(0, 1, &nullSRV);
}

static void CreateRenderTarget(IDXGISwapChain* pSwapChain) {
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer))) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

static void CleanupRenderTarget() {
    ReleaseAmbientCopyResources();
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
            InitAmbientLighting(g_pd3dDevice);

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
        // Native DirectX 11 Ambient Lighting & Color Shader pass directly on BackBuffer
        RenderAmbientLightingPass(pSwapChain);

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
    CleanupAmbientLighting();

    if (g_bInitialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_bInitialized = false;
    }
}

} // namespace DX11Hook
