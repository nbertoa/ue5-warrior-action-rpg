// HeroUIComponent.h
// Hero-specific UI component.
// Extends the base UI component with delegates that drive hero-exclusive
// UI elements — rage bar, combo counter, and any other player-only HUD elements.

#pragma once

#include "CoreMinimal.h"
#include "Components/UI/PawnUIComponent.h"
#include "HeroUIComponent.generated.h"

/**
 * Dynamic multicast delegate broadcast when the hero equips a new weapon.
 * The parameter is a soft reference to the weapon icon texture — the widget
 * is responsible for loading it asynchronously to avoid blocking the game thread.
 *
 * @param SoftWeaponIcon    Soft reference to the equipped weapon's icon texture.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedWeaponChangedDelegate,
                                            TSoftObjectPtr<UTexture2D>,
                                            SoftWeaponIcon);

/**
 * UI component for the hero character.
 * Inherits OnCurrentHealthChanged from UPawnUIComponent and adds
 * hero-specific delegates for attributes that enemies do not have.
 *
 * The attribute set broadcasts these delegates after modifying attributes —
 * widgets bind to them directly without polling or ticking.
 */
UCLASS(ClassGroup=(Custom),
	meta=(BlueprintSpawnableComponent))
class WARRIORRPG_API UHeroUIComponent : public UPawnUIComponent
{
	GENERATED_BODY()

public:
	/**
	 * Broadcast by the attribute set whenever CurrentRage changes.
	 * The value is pre-normalized — listeners receive CurrentRage / MaxRage in [0, 1].
	 * Bind to this in the rage bar widget to update without polling.
	 * Only broadcast for hero characters — enemies do not have a rage attribute.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnPercentChangedDelegate OnCurrentRageChanged;

	/**
	 * Broadcast when the hero equips a new weapon.
	 * Carries a soft reference to the weapon icon texture so the HUD widget
	 * can update the displayed weapon image without knowing the concrete weapon type.
	 * The widget is responsible for async-loading the texture before displaying it.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEquippedWeaponChangedDelegate OnEquippedWeaponChanged;
};
