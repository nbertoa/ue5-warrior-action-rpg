// PawnUIInterface.h
// Interface implemented by any Pawn that owns UI components.
// Provides a uniform way for the attribute set and other systems to retrieve
// UI components without casting to a specific character class.
//
// GetPawnUIComponent() is pure virtual — all implementors must provide it.
// GetHeroUIComponent() and GetEnemyUIComponent() have default no-op implementations —
// only the relevant character subclass overrides each, so the other inherits nullptr.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PawnUIInterface.generated.h"

class UEnemyUIComponent;
class UHeroUIComponent;
class UPawnUIComponent;

/** UObject boilerplate required by UHT to register the interface with the reflection system.
 *  Do not add functions here — implement them in IPawnUIInterface below. */
UINTERFACE(MinimalAPI)
class UPawnUIInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * IPawnUIInterface
 *
 * Implemented by any Pawn that owns a UPawnUIComponent.
 * Provides a uniform way for the attribute set and other systems to retrieve
 * UI components without casting to a specific character class.
 *
 * AWarriorBaseCharacter returns nullptr by default for all three functions.
 * AWarriorHeroCharacter overrides GetPawnUIComponent and GetHeroUIComponent.
 * AWarriorEnemyCharacter overrides GetPawnUIComponent and GetEnemyUIComponent.
 */
class WARRIORRPG_API IPawnUIInterface
{
    GENERATED_BODY()

public:
    /**
     * Returns the base UI component owned by this pawn.
     * Used by the attribute set to broadcast health changes to the UI
     * without knowing the concrete character type.
     *
     * @return The pawn's UPawnUIComponent. Must not return null on any
     *         character that has a UI — pure virtual enforces this contract.
     */
    virtual UPawnUIComponent* GetPawnUIComponent() const = 0;

    /**
     * Returns the hero-specific UI component, or nullptr if this pawn is not a hero.
     * Default implementation returns nullptr — only AWarriorHeroCharacter overrides this.
     * Allows the attribute set to broadcast rage changes to the hero UI without casting.
     *
     * @return The hero's UHeroUIComponent, or nullptr for non-hero pawns.
     */
    virtual UHeroUIComponent* GetHeroUIComponent() const;

    /**
     * Returns the enemy-specific UI component, or nullptr if this pawn is not an enemy.
     * Default implementation returns nullptr — only AWarriorEnemyCharacter overrides this.
     * Allows external systems to access enemy-exclusive UI elements without casting.
     *
     * @return The enemy's UEnemyUIComponent, or nullptr for non-enemy pawns.
     */
    virtual UEnemyUIComponent* GetEnemyUIComponent() const;
};
