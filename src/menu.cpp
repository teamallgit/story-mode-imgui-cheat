#include "menu.h"
#include "dx11_hook.h"
#include <cstdio>
#include <algorithm>
#include <chrono>

CheatSettings g_Settings;

static inline bool IsKeyHeld(int vKey) {
    if (vKey >= 0 && vKey < 256 && g_KeysDown[vKey])
        return true;
    if ((GetAsyncKeyState(vKey) & 0x8000) != 0)
        return true;
    return false;
}

namespace CheatMenu {

void InitializeTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 6.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);

    // Dark cyber/neon FNAF theme
    colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.94f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.55f, 0.65f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.08f, 0.09f, 0.12f, 0.94f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.11f, 0.12f, 0.16f, 0.70f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.10f, 0.11f, 0.15f, 0.96f);
    colors[ImGuiCol_Border]                = ImVec4(0.25f, 0.28f, 0.38f, 0.60f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.18f, 0.24f, 0.80f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.24f, 0.28f, 0.38f, 0.80f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.30f, 0.35f, 0.48f, 0.90f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.12f, 0.08f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.22f, 0.12f, 0.32f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.08f, 0.09f, 0.12f, 0.75f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.12f, 0.13f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.08f, 0.09f, 0.12f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.25f, 0.28f, 0.38f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.35f, 0.40f, 0.52f, 0.90f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.45f, 0.52f, 0.68f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.85f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.75f, 0.25f, 0.45f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.95f, 0.30f, 0.55f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.14f, 0.28f, 0.85f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.32f, 0.20f, 0.45f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.45f, 0.25f, 0.60f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.22f, 0.15f, 0.32f, 0.80f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.35f, 0.22f, 0.50f, 0.90f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.45f, 0.28f, 0.65f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.15f, 0.12f, 0.22f, 0.80f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.35f, 0.22f, 0.50f, 0.90f);
    colors[ImGuiCol_TabSelected]          = ImVec4(0.28f, 0.16f, 0.40f, 1.00f);
}

void Render() {
    if (!g_Settings.bMenuOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(540, 480), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("The Joy of Creation: Reborn - Cheat Menu [Beta 0.1.0]", &g_Settings.bMenuOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.4f, 1.0f), "TJoC: R Mod Menu by Antigravity");
        ImGui::SameLine();
        ImGui::TextDisabled(" | Toggle: [END]");
        ImGui::Separator();

        if (ImGui::BeginTabBar("Tabs")) {
            // ================================================================
            // Tab 1: Visuals & Shaders
            // ================================================================
            if (ImGui::BeginTabItem("  Visuals & Shaders  ")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Lighting & Screen Shaders:");
                ImGui::Spacing();

                ImGui::Checkbox("Fullbright / Brightness Boost", &g_Settings.bFullbright);
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Illuminates pitch-black darkness, hallways, and basement shadows.");
                }

                if (g_Settings.bFullbright) {
                    ImGui::Indent(15.0f);
                    ImGui::SliderFloat("Brightness Level", &g_Settings.FullbrightIntensity, 0.05f, 0.75f, "%.2f");
                    ImGui::Unindent(15.0f);
                }

                ImGui::Spacing();
                const char* shaderNames[] = { "Disabled (Normal)", "Blood Red", "Night Vision Green", "Midnight Blue", "Custom Color" };
                ImGui::Combo("Color Shader", &g_Settings.ColorShader, shaderNames, IM_ARRAYSIZE(shaderNames));
                if (g_Settings.ColorShader > 0) {
                    ImGui::Indent(15.0f);
                    ImGui::SliderFloat("Shader Tint Opacity", &g_Settings.ShaderIntensity, 0.05f, 0.85f, "%.2f");
                    if (g_Settings.ColorShader == 4) { // Custom Color
                        ImGui::ColorEdit3("Tint Color", g_Settings.CustomShaderColor);
                    }
                    ImGui::Unindent(15.0f);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "ESP Visuals:");
                ImGui::Spacing();

                ImGui::Checkbox("Enable Robot ESP", &g_Settings.bRobotESP);
                if (g_Settings.bRobotESP) {
                    ImGui::Indent(15.0f);
                    ImGui::Checkbox("2D Bounding Box", &g_Settings.bBoxESP);
                    ImGui::Checkbox("Robot Name", &g_Settings.bNameESP);
                    ImGui::Checkbox("Distance (Meters)", &g_Settings.bDistanceESP);
                    ImGui::Checkbox("Snaplines (Bottom to Target)", &g_Settings.bSnaplines);
                    ImGui::ColorEdit4("Robot Color", g_Settings.RobotColor, ImGuiColorEditFlags_NoInputs);
                    ImGui::Unindent(15.0f);
                }

                ImGui::Separator();
                ImGui::Checkbox("Enable Objective ESP (Notebooks / Collectibles)", &g_Settings.bCollectibleESP);
                if (g_Settings.bCollectibleESP) {
                    ImGui::Indent(15.0f);
                    ImGui::ColorEdit4("Collectible Color", g_Settings.CollectibleColor, ImGuiColorEditFlags_NoInputs);
                    ImGui::Unindent(15.0f);
                }

                ImGui::Separator();
                ImGui::Checkbox("Display Player Watermark & Coordinates", &g_Settings.bPlayerInfo);

                ImGui::EndTabItem();
            }

            // ================================================================
            // Tab 2: Robot Cheats
            // ================================================================
            if (ImGui::BeginTabItem("  Robot Cheats  ")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Robot Controls & Modifiers:");
                ImGui::Spacing();

                ImGui::Checkbox("Freeze Robots (Stop AI & Animations)", &g_Settings.bFreezeRobots);
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Sets CustomTimeDilation to 0.0f for all robots.\nFreezes them completely in place.");
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("Teleport Robots Away (Push 50m Back)", ImVec2(280, 32))) {
                    g_Settings.bTeleportRobotsAway = true;
                }

                ImGui::Spacing();
                ImGui::Text("Active Robots in Memory: %d", g_Settings.LastRobotCount);

                ImGui::EndTabItem();
            }

            // ================================================================
            // Tab 3: Player Cheats
            // ================================================================
            if (ImGui::BeginTabItem("  Player Cheats  ")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Player Movement & Survival:");
                ImGui::Spacing();

                ImGui::Checkbox("Player NoClip (3D Ghost / Flight)", &g_Settings.bPlayerNoClip);
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Fly & pass through walls, floors, and obstacles!\nControls: WASD / Arrows (Fly), Space (Up), Ctrl/C (Down), Shift (Boost speed).");
                }

                ImGui::Checkbox("God Mode (Jumpscare Immunity & Stealth)", &g_Settings.bGodMode);
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Neutralizes animatronic sight and hearing, and prevents jumpscare collision overlap.\nAnimatronics patrol normally but cannot detect, chase, or kill you!");
                }

                ImGui::Spacing();
                ImGui::Checkbox("Super Sprint Speed", &g_Settings.bSuperSpeed);
                if (g_Settings.bSuperSpeed) {
                    ImGui::SliderFloat("Speed Multiplier", &g_Settings.PlayerSpeedMultiplier, 1.0f, 4.0f, "%.1fx");
                }

                ImGui::EndTabItem();
            }

            // ================================================================
            // Tab 4: Objectives & Map
            // ================================================================
            if (ImGui::BeginTabItem("  Objectives & Info  ")) {
                ImGui::Spacing();
                ImGui::Text("Level Info:");
                ImGui::BulletText("Map Type: Ignited Animatronic Survival");
                ImGui::BulletText("Remaining Collectibles: %d", g_Settings.LastCollectibleCount);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "Mechanics Quick Guide:");
                ImGui::BulletText("Freddy: Listens to footsteps. Crouch to remain silent.");
                ImGui::BulletText("Bonnie: Roams corridors aggressively. Keep moving.");
                ImGui::BulletText("Chica: Senses when you look at her. Look away to freeze.");
                ImGui::BulletText("Foxy: Lurks in shadows. Shine your flashlight to stun!");

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void RenderScreenShaders(ImDrawList* drawList, float screenWidth, float screenHeight) {
    if (!drawList || screenWidth <= 0.0f || screenHeight <= 0.0f) return;

    // 1. Fullbright / Brightness Boost
    if (g_Settings.bFullbright && g_Settings.FullbrightIntensity > 0.01f) {
        float alpha = std::clamp(g_Settings.FullbrightIntensity, 0.0f, 0.85f);
        drawList->AddRectFilled(ImVec2(0, 0), ImVec2(screenWidth, screenHeight), ImColor(1.0f, 1.0f, 1.0f, alpha));
    }

    // 2. Color Shaders (Blood Red, Night Vision Green, Midnight Blue, Custom)
    if (g_Settings.ColorShader > 0 && g_Settings.ShaderIntensity > 0.01f) {
        float alpha = std::clamp(g_Settings.ShaderIntensity, 0.0f, 0.90f);
        ImU32 col = 0;
        switch (g_Settings.ColorShader) {
            case 1: // Blood Red
                col = ImColor(1.0f, 0.05f, 0.05f, alpha);
                break;
            case 2: // Night Vision Green
                col = ImColor(0.08f, 1.0f, 0.15f, alpha);
                break;
            case 3: // Midnight Blue
                col = ImColor(0.1f, 0.35f, 1.0f, alpha);
                break;
            case 4: // Custom Color
                col = ImColor(g_Settings.CustomShaderColor[0], g_Settings.CustomShaderColor[1], g_Settings.CustomShaderColor[2], alpha);
                break;
            default:
                break;
        }

        if (col != 0) {
            drawList->AddRectFilled(ImVec2(0, 0), ImVec2(screenWidth, screenHeight), col);
        }
    }
}

void RenderESP(ImDrawList* drawList, float screenWidth, float screenHeight) {
    if (!drawList) return;

    // Render Fullbright & Screen Color Shaders across entire screen
    RenderScreenShaders(drawList, screenWidth, screenHeight);

    FMinimalViewInfo camera;
    if (!UE4SDK::GetCameraPOV(camera))
        return;

    std::vector<ScannedActor> actors = UE4SDK::ScanActors();

    int robotCount = 0;
    int collectibleCount = 0;

    for (const auto& a : actors) {
        if (a.Type == ActorType::Freddy || a.Type == ActorType::Bonnie ||
            a.Type == ActorType::Chica || a.Type == ActorType::Foxy) {
            robotCount++;

            if (!g_Settings.bRobotESP) continue;

            // Robot foot & head world locations (capsule half height is ~100 units)
            FVector footLoc = a.WorldLocation;
            footLoc.Z -= 100.0f;
            FVector headLoc = a.WorldLocation;
            headLoc.Z += 100.0f;

            float sFootX = 0, sFootY = 0;
            float sHeadX = 0, sHeadY = 0;

            if (UE4SDK::WorldToScreen(footLoc, camera, screenWidth, screenHeight, sFootX, sFootY) &&
                UE4SDK::WorldToScreen(headLoc, camera, screenWidth, screenHeight, sHeadX, sHeadY)) {

                float boxHeight = std::abs(sFootY - sHeadY);
                if (boxHeight < 10.0f) boxHeight = 10.0f;
                float boxWidth = boxHeight * 0.45f;
                float boxLeft = sFootX - (boxWidth / 2.0f);
                float boxTop = std::min(sFootY, sHeadY);
                float boxBottom = std::max(sFootY, sHeadY);

                ImU32 col = ImColor(g_Settings.RobotColor[0], g_Settings.RobotColor[1], g_Settings.RobotColor[2], g_Settings.RobotColor[3]);
                ImU32 colBg = ImColor(0.0f, 0.0f, 0.0f, 0.6f);

                // Snapline from screen bottom center to robot feet
                if (g_Settings.bSnaplines) {
                    ImU32 snapCol = ImColor(g_Settings.SnaplineColor[0], g_Settings.SnaplineColor[1], g_Settings.SnaplineColor[2], g_Settings.SnaplineColor[3]);
                    drawList->AddLine(ImVec2(screenWidth / 2.0f, screenHeight), ImVec2(sFootX, boxBottom), snapCol, 1.2f);
                }

                // 2D Box
                if (g_Settings.bBoxESP) {
                    // Black outline
                    drawList->AddRect(ImVec2(boxLeft - 1, boxTop - 1), ImVec2(boxLeft + boxWidth + 1, boxBottom + 1), colBg, 0.0f, 0, 2.5f);
                    // Colored box
                    drawList->AddRect(ImVec2(boxLeft, boxTop), ImVec2(boxLeft + boxWidth, boxBottom), col, 0.0f, 0, 1.5f);
                }

                // Text: Name & Distance
                char textBuffer[128];
                if (g_Settings.bNameESP && g_Settings.bDistanceESP) {
                    snprintf(textBuffer, sizeof(textBuffer), "%s [%.1fm]", a.DisplayName.c_str(), a.DistanceMeters);
                } else if (g_Settings.bNameESP) {
                    snprintf(textBuffer, sizeof(textBuffer), "%s", a.DisplayName.c_str());
                } else if (g_Settings.bDistanceESP) {
                    snprintf(textBuffer, sizeof(textBuffer), "[%.1fm]", a.DistanceMeters);
                } else {
                    textBuffer[0] = '\0';
                }

                if (textBuffer[0] != '\0') {
                    ImVec2 textSize = ImGui::CalcTextSize(textBuffer);
                    float textX = sFootX - (textSize.x / 2.0f);
                    float textY = boxTop - textSize.y - 4.0f;

                    // Text shadow
                    drawList->AddText(ImVec2(textX + 1, textY + 1), ImColor(0, 0, 0, 255), textBuffer);
                    drawList->AddText(ImVec2(textX, textY), col, textBuffer);
                }
            }
        } else if (a.Type == ActorType::Collectible) {
            collectibleCount++;

            if (!g_Settings.bCollectibleESP) continue;

            float sX = 0, sY = 0;
            if (UE4SDK::WorldToScreen(a.WorldLocation, camera, screenWidth, screenHeight, sX, sY)) {
                ImU32 col = ImColor(g_Settings.CollectibleColor[0], g_Settings.CollectibleColor[1], g_Settings.CollectibleColor[2], g_Settings.CollectibleColor[3]);

                // Small diamond marker
                float dSize = 6.0f;
                drawList->AddQuadFilled(
                    ImVec2(sX, sY - dSize),
                    ImVec2(sX + dSize, sY),
                    ImVec2(sX, sY + dSize),
                    ImVec2(sX - dSize, sY),
                    col
                );
                drawList->AddQuad(
                    ImVec2(sX, sY - dSize),
                    ImVec2(sX + dSize, sY),
                    ImVec2(sX, sY + dSize),
                    ImVec2(sX - dSize, sY),
                    ImColor(0, 0, 0, 255),
                    1.5f
                );

                char label[64];
                snprintf(label, sizeof(label), "Notebook [%.1fm]", a.DistanceMeters);
                drawList->AddText(ImVec2(sX + 8 + 1, sY - 7 + 1), ImColor(0, 0, 0, 255), label);
                drawList->AddText(ImVec2(sX + 8, sY - 7), col, label);
            }
        }
    }

    g_Settings.LastRobotCount = robotCount;
    g_Settings.LastCollectibleCount = collectibleCount;

    // Player Watermark / HUD Overlay
    if (g_Settings.bPlayerInfo) {
        char hud[256];
        snprintf(hud, sizeof(hud), "TJoC: Reborn Mod Menu | Cam: (%.0f, %.0f, %.0f) | Robots: %d | Items: %d",
                 camera.Location.X, camera.Location.Y, camera.Location.Z, robotCount, collectibleCount);
        drawList->AddText(ImVec2(15 + 1, 15 + 1), ImColor(0, 0, 0, 255), hud);
        drawList->AddText(ImVec2(15, 15), ImColor(255, 200, 50, 255), hud);
    }
}

void UpdateCheats() {
    std::vector<ScannedActor> actors = UE4SDK::ScanActors();

    void* playerActor = nullptr;
    FVector playerLoc(0.0f, 0.0f, 0.0f);

    for (const auto& a : actors) {
        if (a.Type == ActorType::Player) {
            playerActor = a.ActorPtr;
            playerLoc = a.WorldLocation;
            break;
        }
    }

    static bool s_bPrevGodMode = false;
    static bool s_bPrevNoClip = false;

    for (const auto& a : actors) {
        if (a.Type == ActorType::Freddy || a.Type == ActorType::Bonnie ||
            a.Type == ActorType::Chica || a.Type == ActorType::Foxy) {

            // Freeze Robots (Stop AI logic & animations in place)
            if (g_Settings.bFreezeRobots) {
                UE4SDK::SetActorTimeDilation(a.ActorPtr, 0.0f);
            } else {
                UE4SDK::SetActorTimeDilation(a.ActorPtr, 1.0f);
            }

            // God Mode: Neutralize PawnSensing (Sight & Hearing) so animatronics never spot, chase, or attack player
            uintptr_t pawnSensing = UE4SDK::FindPawnSensingComponent(a.ActorPtr);
            if (IsValidPtr((void*)pawnSensing)) {
                if (g_Settings.bGodMode) {
                    *(float*)(pawnSensing + Offsets::PawnSensing_HearingThreshold) = 0.0f;
                    *(float*)(pawnSensing + Offsets::PawnSensing_LOSHearingThreshold) = 0.0f;
                    *(float*)(pawnSensing + Offsets::PawnSensing_SightRadius) = 0.0f;
                    *(uint32_t*)(pawnSensing + Offsets::PawnSensing_bEnableSensingUpdates) = 0;
                    *(uint32_t*)(pawnSensing + Offsets::PawnSensing_bHearNoises) = 0;
                    *(float*)(pawnSensing + Offsets::PawnSensing_PeripheralVisionAngle) = 0.0f;
                } else if (s_bPrevGodMode && !g_Settings.bGodMode) {
                    // Restore original sensing defaults when God Mode is toggled off
                    *(float*)(pawnSensing + Offsets::PawnSensing_HearingThreshold) = 1411.89f;
                    *(float*)(pawnSensing + Offsets::PawnSensing_LOSHearingThreshold) = 2747.0f;
                    *(float*)(pawnSensing + Offsets::PawnSensing_SightRadius) = 5000.0f;
                    *(uint32_t*)(pawnSensing + Offsets::PawnSensing_bEnableSensingUpdates) = 0x09;
                    *(uint32_t*)(pawnSensing + Offsets::PawnSensing_bHearNoises) = 0x11;
                    *(float*)(pawnSensing + Offsets::PawnSensing_PeripheralVisionAngle) = 90.0f;
                }
            }

            // God Mode: Prevent physical overlap jumpscare if robot patrols directly into player (buffer at 120 units)
            if (g_Settings.bGodMode && playerActor) {
                FVector toRobot(a.WorldLocation.X - playerLoc.X, a.WorldLocation.Y - playerLoc.Y, 0.0f);
                float dist2D = std::sqrt(toRobot.X * toRobot.X + toRobot.Y * toRobot.Y);
                if (dist2D < 120.0f && dist2D > 0.01f) {
                    toRobot.X /= dist2D;
                    toRobot.Y /= dist2D;
                    FVector safeRobotLoc(playerLoc.X + toRobot.X * 130.0f, playerLoc.Y + toRobot.Y * 130.0f, a.WorldLocation.Z);
                    uintptr_t rRoot = *(uintptr_t*)((uintptr_t)a.ActorPtr + Offsets::Actor_RootComponent);
                    if (IsValidPtr((void*)rRoot)) {
                        *(FVector*)(rRoot + Offsets::SceneComponent_Translation) = safeRobotLoc;
                        *(FVector*)(rRoot + Offsets::SceneComponent_RelativeLocation) = safeRobotLoc;
                        *(FVector*)(rRoot + Offsets::SceneComponent_PhysicsTranslation) = safeRobotLoc;
                    }
                }
            }

            // Teleport Robots Away
            if (g_Settings.bTeleportRobotsAway) {
                FMinimalViewInfo camera;
                if (UE4SDK::GetCameraPOV(camera)) {
                    FVector dir(a.WorldLocation.X - camera.Location.X, a.WorldLocation.Y - camera.Location.Y, 0.0f);
                    float len = dir.Length();
                    if (len > 1.0f) {
                        dir.X /= len;
                        dir.Y /= len;
                        FVector newLoc(camera.Location.X + dir.X * 5000.0f, camera.Location.Y + dir.Y * 5000.0f, a.WorldLocation.Z);
                        uintptr_t root = *(uintptr_t*)((uintptr_t)a.ActorPtr + Offsets::Actor_RootComponent);
                        if (IsValidPtr((void*)root)) {
                            *(FVector*)(root + Offsets::SceneComponent_Translation) = newLoc;
                            *(FVector*)(root + Offsets::SceneComponent_RelativeLocation) = newLoc;
                            *(FVector*)(root + Offsets::SceneComponent_PhysicsTranslation) = newLoc;
                        }
                    }
                }
            }
        } else if (a.Type == ActorType::Player) {
            uintptr_t root = *(uintptr_t*)((uintptr_t)a.ActorPtr + Offsets::Actor_RootComponent);
            uintptr_t moveComp = *(uintptr_t*)((uintptr_t)a.ActorPtr + Offsets::Character_Movement);

            // Speedhack
            if (g_Settings.bSuperSpeed) {
                float mult = g_Settings.PlayerSpeedMultiplier;
                *(float*)((uintptr_t)a.ActorPtr + Offsets::Player_Speed) = 300.0f * mult;
                *(float*)((uintptr_t)a.ActorPtr + Offsets::Player_SprintingSpeed) = 650.0f * mult;
                *(float*)((uintptr_t)a.ActorPtr + Offsets::Player_FastSprintingSpeed) = 700.0f * mult;
                *(float*)((uintptr_t)a.ActorPtr + Offsets::Actor_CustomTimeDilation) = mult;
                if (IsValidPtr((void*)moveComp)) {
                    *(float*)(moveComp + Offsets::Movement_MaxWalkSpeed) = 600.0f * mult;
                }
            } else {
                *(float*)((uintptr_t)a.ActorPtr + Offsets::Player_Speed) = 300.0f;
                *(float*)((uintptr_t)a.ActorPtr + Offsets::Player_SprintingSpeed) = 650.0f;
                *(float*)((uintptr_t)a.ActorPtr + Offsets::Player_FastSprintingSpeed) = 700.0f;
                *(float*)((uintptr_t)a.ActorPtr + Offsets::Actor_CustomTimeDilation) = 1.0f;
                if (IsValidPtr((void*)moveComp)) {
                    *(float*)(moveComp + Offsets::Movement_MaxWalkSpeed) = 600.0f;
                }
            }

            // Player NoClip (3D Ghost / Flight through walls)
            if (g_Settings.bPlayerNoClip) {
                // 1. Disable collision on player so they pass through all geometry/walls
                UE4SDK::SetActorCollision(a.ActorPtr, false);

                // 2. Set MovementMode to MOVE_Flying (5) to disable gravity and floor snapping
                if (IsValidPtr((void*)moveComp)) {
                    *(uint8_t*)(moveComp + Offsets::Movement_MovementMode) = 5;
                }

                // 3. Process 3D flight movement when cheat menu is closed
                if (!g_Settings.bMenuOpen && IsValidPtr((void*)root)) {
                    FMinimalViewInfo camera;
                    if (UE4SDK::GetCameraPOV(camera)) {
                        FVector vForward, vRight, vUp;
                        UE4SDK::GetAxes(camera.Rotation, vForward, vRight, vUp);

                        FVector moveDir(0.0f, 0.0f, 0.0f);
                        if (IsKeyHeld('W') || IsKeyHeld(VK_UP)) {
                            moveDir.X += vForward.X; moveDir.Y += vForward.Y; moveDir.Z += vForward.Z;
                        }
                        if (IsKeyHeld('S') || IsKeyHeld(VK_DOWN)) {
                            moveDir.X -= vForward.X; moveDir.Y -= vForward.Y; moveDir.Z -= vForward.Z;
                        }
                        if (IsKeyHeld('D') || IsKeyHeld(VK_RIGHT)) {
                            moveDir.X += vRight.X; moveDir.Y += vRight.Y; moveDir.Z += vRight.Z;
                        }
                        if (IsKeyHeld('A') || IsKeyHeld(VK_LEFT)) {
                            moveDir.X -= vRight.X; moveDir.Y -= vRight.Y; moveDir.Z -= vRight.Z;
                        }
                        if (IsKeyHeld(VK_SPACE)) {
                            moveDir.Z += 1.0f;
                        }
                        if (IsKeyHeld(VK_CONTROL) || IsKeyHeld('C')) {
                            moveDir.Z -= 1.0f;
                        }

                        float len = moveDir.Length();
                        if (len > 0.001f) {
                            moveDir.X /= len; moveDir.Y /= len; moveDir.Z /= len;

                            float speed = 1000.0f;
                            if (IsKeyHeld(VK_SHIFT)) {
                                speed *= 2.5f;
                            }
                            if (g_Settings.bSuperSpeed) {
                                speed *= g_Settings.PlayerSpeedMultiplier;
                            }

                            static auto s_lastTick = std::chrono::high_resolution_clock::now();
                            auto now = std::chrono::high_resolution_clock::now();
                            float dt = std::chrono::duration<float>(now - s_lastTick).count();
                            s_lastTick = now;
                            if (dt <= 0.0f || dt > 0.1f) dt = 0.0166f;

                            FVector curLoc = *(FVector*)(root + Offsets::SceneComponent_Translation);
                            curLoc.X += moveDir.X * speed * dt;
                            curLoc.Y += moveDir.Y * speed * dt;
                            curLoc.Z += moveDir.Z * speed * dt;

                            *(FVector*)(root + Offsets::SceneComponent_Translation) = curLoc;
                            *(FVector*)(root + Offsets::SceneComponent_RelativeLocation) = curLoc;
                            *(FVector*)(root + Offsets::SceneComponent_PhysicsTranslation) = curLoc;
                        }
                    }
                }
            } else if (s_bPrevNoClip && !g_Settings.bPlayerNoClip) {
                // Restore collision and MovementMode to MOVE_Walking (1) when noclip is turned off
                UE4SDK::SetActorCollision(a.ActorPtr, true);
                if (IsValidPtr((void*)moveComp)) {
                    *(uint8_t*)(moveComp + Offsets::Movement_MovementMode) = 1;
                }
            }
        }
    }

    s_bPrevGodMode = g_Settings.bGodMode;
    s_bPrevNoClip = g_Settings.bPlayerNoClip;
    g_Settings.bTeleportRobotsAway = false;
}

} // namespace CheatMenu
