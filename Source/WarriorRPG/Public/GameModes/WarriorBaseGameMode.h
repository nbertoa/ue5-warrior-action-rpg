// WarriorBaseGameMode.h
// Base game mode for the WarriorRPG project.
// Defines default rules for player spawning, pawn class, and match flow.
// Extend this class for specific game mode variants (e.g., dungeon, arena, lobby).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WarriorBaseGameMode.generated.h"

/**
 * Base game mode for WarriorRPG.
 * Serves as the foundation for all game mode variants in the project.
 * Configure default pawn class, player controller, and HUD in the Blueprint child.
 */
UCLASS()
class WARRIORRPG_API AWarriorBaseGameMode : public AGameModeBase
{
    GENERATED_BODY()
};
