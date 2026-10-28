# The Joy of Creation: Reborn - Story Mode ImGui Cheat Menu

A complete reverse-engineered DirectX 11 ImGui cheat menu and mechanics modifier for **The Joy of Creation: Reborn (Beta 0.1.0)**. Works on both **Windows** and **Linux / Steam Deck (Proton / Wine)** without requiring third-party injectors.

---

## Features

### 1. Visuals & ESP
- **Robot ESP**:
  - **2D Bounding Boxes**: Adaptive screen-space bounding boxes surrounding all active Ignited animatronics.
  - **Names**: Clear labels (`Ignited Freddy`, `Ignited Bonnie`, `Ignited Chica`, `Ignited Foxy`).
  - **Real-Time Distance**: Precise distance in meters (derived from Unreal Engine 4 world units: 100 units = 1 meter).
  - **Snaplines**: Directional line from screen bottom to robot location.
  - **Customizable Colors**: RGB color picker for robot markers.
- **Objective ESP (Notebooks)**:
  - 3D diamond markers through walls for all scattered **Notebooks (`BP_Collectible`)** with exact distance readouts.
- **Player HUD Overlay**:
  - Live 3D camera coordinates $(X, Y, Z)$ and counts of detected animatronics and remaining collectibles.

### 2. In-Engine Ambient Lighting & Color
- **True In-Engine Lighting / Fullbright Boost**:
  - Dynamically amplifies native Unreal Engine 3D light sources (`SkyLight`, `PointLight`, `SpotLight`, room lamps, and the player flashlight).
  - Illuminates pitch-black darkness, hallways, and basement shadows in full 3D with realistic diffuse and specular bounce—**not** a flat 2D gamma or screen rectangle overlay!
  - Adjustable brightness slider (1.0x to 10.0x).
- **Ambient Light Color Tinting**:
  - Changes the actual emitted color of ambient and direct light in the 3D scene:
    - **Natural Bright White**: Crisp, realistic fullbright clarity.
    - **Blood Red**: Atmospheric crimson / ignited horror ambient glow.
    - **Toxic Green**: Night-vision style eerie ambient lighting.
    - **Deep Blue**: Midnight cinematic moody blue atmosphere.
    - **Neon Purple**: Cyberpunk neon ambient glow.
    - **Custom Color**: RGB color picker to illuminate the scene in any custom light color!

### 3. Player Movement & Survival
- **Player NoClip (Pass Through Walls)**:
  - Phase smoothly through all walls, doors, ceilings, and barriers.
  - Disables collision on the player character, allowing seamless passage through any obstacle.
  - Toggling off restores normal ground walking collision immediately.
- **God Mode (Stealth & Jumpscare Immunity)**:
  - Neutralizes `UPawnSensingComponent` (sight radius, hearing threshold, noise sensing flags).
  - Animatronics continue their normal patrol animations and walking paths, but are 100% blind and deaf to the player.
  - Includes a physical overlap proximity clamp to prevent `ReceiveActorBeginOverlap` jumpscares even if you walk directly into an animatronic.
- **Super Speedhack**:
  - Multiplier slider (1.0x to 4.0x) scaling walking speed, sprinting speed, fast sprinting speed, and actor time dilation.

### 4. Animatronic Cheats
- **Freeze Robots**: Sets `CustomTimeDilation = 0.0f` on all animatronics, freezing their animations and AI logic completely in place.
- **Teleport Robots Away**: Pushes animatronics 50 meters away along your line of sight.

---

## Controls

| Key | Action |
| :--- | :--- |
| **`[END]`** / `[INSERT]` | Open / Close Cheat Menu |
| **`WASD`** / Arrows | Fly in camera look direction (when NoClip is active) |
| **`Space`** | Fly Upwards |
| **`Ctrl`** / **`C`** | Fly Downwards |
| **`Shift`** | Boost Flight Speed (2.5x) |

---

## Cross-Installation Guide

Pre-compiled DLLs are included in the [`bin/`](bin/) folder so you can install this onto any TJoC: R installation without compiling.

### Method 1: Drop-in Proxy DLL (Recommended - Zero Injector Needed)

The game's 64-bit Unreal Engine executable (`UE4Game-Win64-Shipping.exe`) dynamically links to `xinput1_3.dll`. By placing our proxy DLL in the same directory, the game automatically loads the cheat menu upon launch.

1. Locate your game directory:
   `<Game Root>/Engine/Binaries/Win64/`
2. Copy **`bin/xinput1_3.dll`** into:
   `<Game Root>/Engine/Binaries/Win64/xinput1_3.dll`
3. Launch the game normally and press **`[END]`**!

#### Automated Scripts:
- **Linux / Steam Deck**:
  ```bash
  ./install.sh "/path/to/TJoC/Engine/Binaries/Win64"
  ```
- **Windows**:
  Run `install.bat` and follow the prompt, or drag and drop your `Win64` folder onto `install.bat`.

### Method 2: DXGI Proxy DLL
Alternatively, copy **`bin/dxgi.dll`** into `<Game Root>/Engine/Binaries/Win64/dxgi.dll`.

### Method 3: Standalone Injector DLL
If you prefer manual injection, use **`bin/tjoc_cheat.dll`** with any standard x64 DLL injector (e.g., Xenos, Cheat Engine, Process Hacker) targeting `UE4Game-Win64-Shipping.exe`.

---

## Building from Source

### On Linux (Cross-compiling with MinGW-w64)
```bash
./build.sh
```
Build outputs are written directly to `bin/`.

### Dependencies (Included in Repo)
- **MinHook**: x64 API hooking library for DirectX 11 Present & ResizeBuffers.
- **Dear ImGui**: Modern UI overlay and drawing list.

---

## Project Structure
```
├── bin/                 # Pre-compiled binaries (xinput1_3.dll, dxgi.dll, tjoc_cheat.dll)
├── src/
│   ├── dx11_hook.cpp    # DirectX 11 hook, swapchain present, RawInput/WndProc handler
│   ├── dx11_hook.h
│   ├── menu.cpp         # Cheat menu UI, ESP renderer, fullbright & shader overlays
│   ├── menu.h
│   ├── sdk.h            # UE4 reverse engineered structures, offsets, and actor scanner
│   ├── proxy_xinput.cpp # XInput proxy exports
│   ├── proxy_dxgi.cpp   # DXGI proxy exports
│   └── dllmain.cpp      # DLL entry point and initialization thread
├── imgui/               # Dear ImGui library & backends
├── minhook/             # MinHook hooking library
├── build.sh             # Linux build script
├── install.sh           # Linux cross-installation script
├── install.bat          # Windows cross-installation script
└── README.md
```

---

## Disclaimer
This mod menu is designed exclusively for offline, single-player reverse engineering and modding of *The Joy of Creation: Reborn*. All game assets, names, and characters belong to Nikson and Scott Cawthon.
