#pragma once
#include <windows.h>
#include <vector>
#include "sdk.h"
#include "../imgui/imgui.h"

struct CheatSettings {
    bool bMenuOpen = true;

    // ESP
    bool bRobotESP = true;
    bool bBoxESP = true;
    bool bNameESP = true;
    bool bDistanceESP = true;
    bool bSnaplines = true;
    bool bCollectibleESP = true;
    bool bPlayerInfo = true;

    // Colors
    float RobotColor[4] = { 1.0f, 0.2f, 0.2f, 1.0f };       // Bright Red
    float CollectibleColor[4] = { 0.2f, 0.85f, 1.0f, 1.0f }; // Neon Cyan
    float SnaplineColor[4] = { 1.0f, 0.5f, 0.0f, 0.7f };    // Orange

    // Lighting & Screen Shaders
    bool bFullbright = false;
    float FullbrightIntensity = 0.35f;
    int ColorShader = 0; // 0=None, 1=Red, 2=Green, 3=Blue, 4=Custom
    float ShaderIntensity = 0.35f;
    float CustomShaderColor[4] = { 1.0f, 0.0f, 0.5f, 0.35f };

    // Robot Cheats
    bool bFreezeRobots = false;       // Freezes robot logic & animations in place
    bool bTeleportRobotsAway = false; // Teleports robots 100 meters away

    // Player Cheats
    bool bPlayerNoClip = false;       // Player passes through walls & 3D flies
    bool bGodMode = false;            // Jumpscare immunity (neutralizes sensory detection)
    bool bSuperSpeed = false;
    float PlayerSpeedMultiplier = 1.5f;

    // Runtime state
    int LastRobotCount = 0;
    int LastCollectibleCount = 0;
    std::string CurrentLevelName = "Detecting...";
};

extern CheatSettings g_Settings;

namespace CheatMenu {
    void InitializeTheme();
    void Render();
    void RenderESP(ImDrawList* drawList, float screenWidth, float screenHeight);
    void UpdateCheats();
}
