// PawnUIComponent.h
// Base UI component shared by all characters in WarriorRPG (hero and enemies).
// Declares delegates that the attribute set broadcasts to drive UI updates
// without the attribute set needing to know about any specific widget class.
// Subclass this per character type to add faction-specific UI delegates.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnExtensionComponentBase.h"
#include "PawnUIComponent.generated.h"

/**
 * Dynamic multicast delegate broadcast when a percentage-based attribute changes.
 * The single parameter is the new normalized value in [0, 1] — callers divide
 * current by max before broadcasting so widgets never need to know the raw values.
 *
 * @param NewPercent    The new attribute value normalized to [0, 1].
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPercentChangedDelegate,
                                            float,
                                            NewPercent);

/**
 * Base UI component for WarriorRPG Pawns.
 * Owns the delegates that drive common UI elements shared by all character types.
 * The attribute set broadcasts these delegates after modifying attributes —
 * widgets bind to them directly without polling or ticking.
 *
 * Subclass this for faction-specific UI (UHeroUIComponent adds rage,
 * UEnemyUIComponent can add threat or phase indicators).
 */
UCLASS(ClassGroup=(Custom),
	meta=(BlueprintSpawnableComponent))
class WARRIORRPG_API UPawnUIComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:
	/**
	 * Broadcast by the attribute set whenever CurrentHealth changes.
	 * The value is pre-normalized — listeners receive CurrentHealth / MaxHealth in [0, 1].
	 * Bind to this in the health bar widget to update without polling.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnPercentChangedDelegate OnCurrentHealthChanged;
};
