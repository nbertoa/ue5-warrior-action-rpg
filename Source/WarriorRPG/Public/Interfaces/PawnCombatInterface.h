#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PawnCombatInterface.generated.h"

class UPawnCombatComponent;

/** UObject boilerplate required by UHT to register the interface with the reflection system.
 *  Do not add functions here — implement them in IPawnCombatInterface below. */
UINTERFACE(MinimalAPI)
class UPawnCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IPawnCombatInterface
 *
 * Implemented by any Pawn that owns a UPawnCombatComponent.
 * Provides a uniform way for abilities and utilities to retrieve the combat
 * component without casting to a specific character class.
 *
 * AWarriorBaseCharacter returns nullptr by default.
 * AWarriorHeroCharacter returns UHeroCombatComponent.
 * AWarriorEnemyCharacter returns UEnemyCombatComponent.
 */
class WARRIORRPG_API IPawnCombatInterface
{
	GENERATED_BODY()

public:
	/** Returns the combat component owned by this pawn, or nullptr if none exists. */
	virtual UPawnCombatComponent* GetPawnCombatComponent() const = 0;
};
