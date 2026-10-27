#pragma once
#include <windows.h>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

// ============================================================================
// Unreal Engine 4.12 Math & Core Structures
// Reverse engineered for The Joy of Creation: Reborn (Shipping x64)
// ============================================================================

#define DEG2RAD 0.017453292519943295f

inline bool IsValidPtr(const void* p) {
    uintptr_t u = (uintptr_t)p;
    return (u >= 0x10000 && u < 0x00007FFFFFFFFFFFULL);
}

struct FVector {
    float X, Y, Z;

    FVector() : X(0.0f), Y(0.0f), Z(0.0f) {}
    FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

    inline float Distance(const FVector& other) const {
        float dx = X - other.X;
        float dy = Y - other.Y;
        float dz = Z - other.Z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    inline float Length() const {
        return std::sqrt(X * X + Y * Y + Z * Z);
    }
};

struct FRotator {
    float Pitch, Yaw, Roll;

    FRotator() : Pitch(0.0f), Yaw(0.0f), Roll(0.0f) {}
    FRotator(float p, float y, float r) : Pitch(p), Yaw(y), Roll(r) {}
};

struct FMinimalViewInfo {
    FVector Location;
    FRotator Rotation;
    float FOV;
    float DesiredFOV;
    float OrthoWidth;
    float OrthoNearClipPlane;
    float OrthoFarClipPlane;
    float AspectRatio;
};

struct FName {
    int32_t ComparisonIndex;
    int32_t Number;
};

struct FNameEntry {
    int32_t Index;
    FNameEntry* HashNext;
    union {
        char AnsiName[1024];
        wchar_t WideName[1024];
    };
};

struct TNameEntryArray {
    FNameEntry** Chunks[128];
    int32_t NumElements;
    int32_t NumChunks;
};

// ============================================================================
// Verified Game Offsets (TJoC: R Beta 0.1.0 UE4 4.12 Shipping)
// ============================================================================
namespace Offsets {
    constexpr uintptr_t GWorld = 0x251da60;
    constexpr uintptr_t GEngine = 0x2518790;
    constexpr uintptr_t GNames = 0x24FB478;
    constexpr uintptr_t GUObjectArray = 0x23EDA80;
    constexpr uintptr_t SetActorEnableCollision = 0xA734F0;

    // GEngine
    constexpr uintptr_t Engine_GameViewport = 0x600;

    // UGameViewportClient
    constexpr uintptr_t Viewport_World = 0x80;

    // UWorld
    constexpr uintptr_t World_PersistentLevel = 0x30;
    constexpr uintptr_t World_OwningGameInstance = 0x120;

    // ULevel
    constexpr uintptr_t Level_Actors = 0xA0;
    constexpr uintptr_t Level_ActorsCount = 0xA8;

    // UGameInstance
    constexpr uintptr_t GameInstance_LocalPlayers = 0x38;

    // ULocalPlayer
    constexpr uintptr_t LocalPlayer_PlayerController = 0x30;

    // APlayerController
    constexpr uintptr_t PlayerController_AcknowledgedPawn = 0x398;
    constexpr uintptr_t PlayerController_PlayerCameraManager = 0x418;

    // APlayerCameraManager
    constexpr uintptr_t CameraManager_CameraCachePOV = 0x3F8;

    // AActor
    constexpr uintptr_t Actor_RootComponent = 0x160;
    constexpr uintptr_t Actor_CustomTimeDilation = 0x78;

    // USceneComponent
    constexpr uintptr_t SceneComponent_Translation = 0x120;
    constexpr uintptr_t SceneComponent_RelativeLocation = 0x160;
    constexpr uintptr_t SceneComponent_PhysicsTranslation = 0xF4;

    // FreeRoamPlayerCharacter blueprint variables
    constexpr uintptr_t Player_Speed = 0x7D8;
    constexpr uintptr_t Player_SprintingSpeed = 0x820;
    constexpr uintptr_t Player_FastSprintingSpeed = 0x824;

    // ACharacter
    constexpr uintptr_t Character_Movement = 0x3E8;

    // UMovementComponent / UCharacterMovementComponent
    constexpr uintptr_t Movement_Velocity = 0x110;
    constexpr uintptr_t Movement_MovementMode = 0x180;
    constexpr uintptr_t Movement_MaxWalkSpeed = 0x1B0;

    // UPawnSensingComponent
    constexpr uintptr_t PawnSensing_HearingThreshold = 0xC8;
    constexpr uintptr_t PawnSensing_LOSHearingThreshold = 0xCC;
    constexpr uintptr_t PawnSensing_SightRadius = 0xD0;
    constexpr uintptr_t PawnSensing_SensingInterval = 0xD4;
    constexpr uintptr_t PawnSensing_HearingMaxSoundAge = 0xD8;
    constexpr uintptr_t PawnSensing_bEnableSensingUpdates = 0xDC;
    constexpr uintptr_t PawnSensing_bHearNoises = 0xE0;
    constexpr uintptr_t PawnSensing_PeripheralVisionAngle = 0x108;
}

// Function pointer for AActor::SetActorEnableCollision
typedef void (*tSetActorEnableCollision)(void* Actor, bool bNewActorEnableCollision);

// Actor classification
enum class ActorType {
    Unknown,
    Freddy,
    Bonnie,
    Chica,
    Foxy,
    Player,
    Collectible
};

struct ScannedActor {
    void* ActorPtr;
    ActorType Type;
    std::string Name;
    std::string DisplayName;
    FVector WorldLocation;
    float DistanceMeters;
    bool bCollisionDisabled;
    bool bFrozen;
};

// ============================================================================
// SDK & Memory Helper Class
// ============================================================================
class UE4SDK {
public:
    static uintptr_t GetBaseAddress() {
        static uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
        return base;
    }

    static tSetActorEnableCollision GetSetCollisionFn() {
        static tSetActorEnableCollision fn = (tSetActorEnableCollision)(GetBaseAddress() + Offsets::SetActorEnableCollision);
        return fn;
    }

    static uintptr_t GetWorld() {
        uintptr_t base = GetBaseAddress();

        // 1. Direct GWorld pointer
        uintptr_t gWorldPtr = *(uintptr_t*)(base + Offsets::GWorld);
        if (IsValidPtr((void*)gWorldPtr)) {
            uintptr_t level = *(uintptr_t*)(gWorldPtr + Offsets::World_PersistentLevel);
            if (IsValidPtr((void*)level)) {
                return gWorldPtr;
            }
        }

        // 2. Fallback via GEngine -> GameViewport -> World
        uintptr_t gEngine = *(uintptr_t*)(base + Offsets::GEngine);
        if (IsValidPtr((void*)gEngine)) {
            uintptr_t viewport = *(uintptr_t*)(gEngine + Offsets::Engine_GameViewport);
            if (IsValidPtr((void*)viewport)) {
                uintptr_t world = *(uintptr_t*)(viewport + Offsets::Viewport_World);
                if (IsValidPtr((void*)world)) {
                    return world;
                }
            }
        }

        return 0;
    }

    static std::string GetNameFromIndex(int32_t index) {
        if (index < 0) return "";
        uintptr_t gNamesPtr = *(uintptr_t*)(GetBaseAddress() + Offsets::GNames);
        if (!IsValidPtr((void*)gNamesPtr)) return "";

        int32_t chunkIndex = index / 16384;
        int32_t withinChunk = index % 16384;

        uintptr_t* chunkTable = (uintptr_t*)gNamesPtr;
        if (!IsValidPtr((void*)chunkTable[chunkIndex])) return "";

        uintptr_t entryAddr = ((uintptr_t*)chunkTable[chunkIndex])[withinChunk];
        if (!IsValidPtr((void*)entryAddr)) return "";

        FNameEntry* entry = (FNameEntry*)entryAddr;
        if (entry->Index & 1) { // Wide string
            wchar_t* wide = (wchar_t*)((uintptr_t)entry + 0x10);
            char buffer[256] = { 0 };
            WideCharToMultiByte(CP_UTF8, 0, wide, -1, buffer, sizeof(buffer), NULL, NULL);
            return std::string(buffer);
        } else {
            char* ansi = (char*)((uintptr_t)entry + 0x10);
            return std::string(ansi);
        }
    }

    static std::string GetActorName(void* actor) {
        if (!IsValidPtr(actor)) return "";
        FName name = *(FName*)((uintptr_t)actor + 0x18);
        return GetNameFromIndex(name.ComparisonIndex);
    }

    static bool GetActorLocation(void* actor, FVector& outLoc) {
        if (!IsValidPtr(actor)) return false;
        uintptr_t rootComp = *(uintptr_t*)((uintptr_t)actor + Offsets::Actor_RootComponent);
        if (!IsValidPtr((void*)rootComp)) return false;
        outLoc = *(FVector*)(rootComp + Offsets::SceneComponent_Translation);
        return true;
    }

    static bool SetActorCollision(void* actor, bool enable) {
        if (!IsValidPtr(actor)) return false;
        tSetActorEnableCollision fn = GetSetCollisionFn();
        if (fn) {
            fn(actor, enable);
            return true;
        }
        return false;
    }

    static bool SetActorTimeDilation(void* actor, float dilation) {
        if (!IsValidPtr(actor)) return false;
        *(float*)((uintptr_t)actor + Offsets::Actor_CustomTimeDilation) = dilation;
        return true;
    }

    static uintptr_t FindPawnSensingComponent(void* actor) {
        if (!IsValidPtr(actor)) return 0;
        // Direct offset first (+0x798)
        uintptr_t direct = *(uintptr_t*)((uintptr_t)actor + 0x798);
        if (IsValidPtr((void*)direct)) {
            std::string name = GetActorName((void*)direct);
            if (name.find("PawnSensing") != std::string::npos || name.find("Sensing") != std::string::npos) {
                return direct;
            }
        }
        // Scan actor candidate pointers
        for (uintptr_t off = 0x300; off < 0x850; off += sizeof(uintptr_t)) {
            uintptr_t p = *(uintptr_t*)((uintptr_t)actor + off);
            if (IsValidPtr((void*)p) && (p & 0x7) == 0) {
                std::string name = GetActorName((void*)p);
                if (name.find("PawnSensing") != std::string::npos || name.find("Sensing") != std::string::npos) {
                    return p;
                }
            }
        }
        return 0;
    }

    static bool GetCameraPOV(FMinimalViewInfo& outPOV) {
        uintptr_t world = GetWorld();
        if (!world) return false;

        uintptr_t gameInst = *(uintptr_t*)(world + Offsets::World_OwningGameInstance);
        if (!IsValidPtr((void*)gameInst)) return false;

        uintptr_t localPlayers = *(uintptr_t*)(gameInst + Offsets::GameInstance_LocalPlayers);
        if (!IsValidPtr((void*)localPlayers)) return false;

        uintptr_t firstPlayer = *(uintptr_t*)localPlayers;
        if (!IsValidPtr((void*)firstPlayer)) return false;

        uintptr_t controller = *(uintptr_t*)(firstPlayer + Offsets::LocalPlayer_PlayerController);
        if (!IsValidPtr((void*)controller)) return false;

        uintptr_t camManager = *(uintptr_t*)(controller + Offsets::PlayerController_PlayerCameraManager);
        if (!IsValidPtr((void*)camManager)) return false;

        outPOV = *(FMinimalViewInfo*)(camManager + Offsets::CameraManager_CameraCachePOV);
        return (outPOV.FOV > 10.0f && outPOV.FOV < 170.0f);
    }

    static void GetAxes(const FRotator& rot, FVector& X, FVector& Y, FVector& Z) {
        float radPitch = rot.Pitch * DEG2RAD;
        float radYaw   = rot.Yaw   * DEG2RAD;
        float radRoll  = rot.Roll  * DEG2RAD;

        float SP = std::sin(radPitch), CP = std::cos(radPitch);
        float SY = std::sin(radYaw),   CY = std::cos(radYaw);
        float SR = std::sin(radRoll),  CR = std::cos(radRoll);

        X = FVector(CP * CY, CP * SY, SP);
        Y = FVector(CY * SP * SR - CR * SY, SY * SP * SR + CR * CY, -CP * SR);
        Z = FVector(-(CY * SP * CR + SR * SY), CY * SR - SY * SP * CR, CP * CR);
    }

    static bool WorldToScreen(const FVector& worldLoc, const FMinimalViewInfo& camera, float screenWidth, float screenHeight, float& screenX, float& screenY) {
        FVector delta = FVector(worldLoc.X - camera.Location.X, worldLoc.Y - camera.Location.Y, worldLoc.Z - camera.Location.Z);

        FVector vAxisX, vAxisY, vAxisZ;
        GetAxes(camera.Rotation, vAxisX, vAxisY, vAxisZ);

        FVector transformed(
            delta.X * vAxisY.X + delta.Y * vAxisY.Y + delta.Z * vAxisY.Z,
            delta.X * vAxisZ.X + delta.Y * vAxisZ.Y + delta.Z * vAxisZ.Z,
            delta.X * vAxisX.X + delta.Y * vAxisX.Y + delta.Z * vAxisX.Z
        );

        if (transformed.Z < 1.0f)
            return false;

        float centerX = screenWidth / 2.0f;
        float centerY = screenHeight / 2.0f;
        float fovRad = camera.FOV * DEG2RAD;
        float tanHalfFOV = std::tan(fovRad * 0.5f);

        screenX = centerX + transformed.X * (centerX / tanHalfFOV) / transformed.Z;
        screenY = centerY - transformed.Y * (centerX / tanHalfFOV) / transformed.Z;

        return (screenX >= -200.0f && screenX <= screenWidth + 200.0f && screenY >= -200.0f && screenY <= screenHeight + 200.0f);
    }

    static std::vector<ScannedActor> ScanActors() {
        std::vector<ScannedActor> result;
        uintptr_t world = GetWorld();
        if (!world) return result;

        uintptr_t level = *(uintptr_t*)(world + Offsets::World_PersistentLevel);
        if (!IsValidPtr((void*)level)) return result;

        uintptr_t actorList = *(uintptr_t*)(level + Offsets::Level_Actors);
        int32_t count = *(int32_t*)(level + Offsets::Level_ActorsCount);

        if (!IsValidPtr((void*)actorList) || count <= 0 || count > 50000)
            return result;

        FMinimalViewInfo camera;
        bool hasCam = GetCameraPOV(camera);

        void** actors = (void**)actorList;
        for (int32_t i = 0; i < count; i++) {
            void* actor = actors[i];
            if (!IsValidPtr(actor)) continue;

            std::string name = GetActorName(actor);
            if (name.empty()) continue;

            ActorType type = ActorType::Unknown;
            std::string displayName;

            if (name.find("BP_Freddy") != std::string::npos) {
                type = ActorType::Freddy;
                displayName = "Ignited Freddy";
            } else if (name.find("BP_Bonnie") != std::string::npos) {
                type = ActorType::Bonnie;
                displayName = "Ignited Bonnie";
            } else if (name.find("BP_Chica") != std::string::npos) {
                type = ActorType::Chica;
                displayName = "Ignited Chica";
            } else if (name.find("BP_Foxy") != std::string::npos) {
                type = ActorType::Foxy;
                displayName = "Ignited Foxy";
            } else if (name.find("FreeRoamPlayerCharacter") != std::string::npos) {
                type = ActorType::Player;
                displayName = "Player";
            } else if (name.find("BP_Collectible") != std::string::npos) {
                type = ActorType::Collectible;
                displayName = "Notebook";
            }

            if (type != ActorType::Unknown) {
                FVector loc;
                if (GetActorLocation(actor, loc)) {
                    float dist = hasCam ? (loc.Distance(camera.Location) / 100.0f) : 0.0f;
                    result.push_back({ actor, type, name, displayName, loc, dist, false, false });
                }
            }
        }
        return result;
    }
};
