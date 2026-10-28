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

    // In-Engine Ambient Lighting & Color
    bool bAmbientLighting = false;        // Real in-game ambient lighting & brightness boost
    float AmbientBrightness = 3.0f;       // Brightness multiplier (1.0x - 10.0x)
    bool bAmbientColor = false;           // Change ambient light color in the game
    int AmbientColorMode = 0;             // 0=Natural White, 1=Blood Red, 2=Toxic Green, 3=Deep Blue, 4=Neon Purple, 5=Custom
    float CustomAmbientColor[3] = { 1.0f, 1.0f, 1.0f }; // Custom RGB color

    // Robot Cheats
    bool bFreezeRobots = false;       // Freezes robot logic & animations in place
    bool bTeleportRobotsAway = false; // Teleports robots 100 meters away

    // Player Cheats
    bool bPlayerNoClip = false;       // Player passes through walls and barriers
    bool bGodMode = false;            // Jumpscare immunity (neutralizes sensory detection)
    bool bSuperSpeed = false;
    float PlayerSpeedMultiplier = 1.5f;

    // Runtime state
    int LastRobotCount = 0;
    int LastCollectibleCount = 0;
    int ActiveLightCount = 0;
    std::string CurrentLevelName = "Detecting...";
};

extern CheatSettings g_Settings;

namespace CheatMenu {
    void InitializeTheme();
    void Render();
    void RenderESP(ImDrawList* drawList, float screenWidth, float screenHeight);
    void UpdateCheats();
}
