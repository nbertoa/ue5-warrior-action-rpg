// EnemyUIComponent.h
// Enemy-specific UI component.
// Extends the base UI component with any delegates that drive enemy-exclusive
// UI elements — health bars above enemy heads, threat indicators, phase markers.

#pragma once

#include "CoreMinimal.h"
#include "Components/UI/PawnUIComponent.h"
#include "EnemyUIComponent.generated.h"

class UWarriorActivatableWidget;

/**
 * UI component for AI-controlled enemy characters.
 * Inherits OnCurrentHealthChanged from UPawnUIComponent for driving
 * overhead health bars and any other shared UI elements.
 *
 * Tracks all widgets drawn by this enemy instance so they can be
 * cleaned up when the enemy dies or despawns, preventing dangling
 * widgets from remaining in the world after the owner is destroyed.
 *
 * Enemy-specific UI delegates (threat level, aggro state, phase transitions)
 * will be added here as the enemy UI system develops.
 */
UCLASS(ClassGroup=(Custom),
	meta=(BlueprintSpawnableComponent))
class WARRIORRPG_API UEnemyUIComponent : public UPawnUIComponent
{
	GENERATED_BODY()

public:
	/**
	 * Registers a widget as drawn by this enemy instance.
	 * Called when a widget is created and attached to this enemy so it
	 * can be tracked for cleanup on death or despawn.
	 *
	 * @param InWidget  The widget to register. Must not be null.
	 */
	UFUNCTION(BlueprintCallable)
	void RegisterEnemyDrawnWidget(UWarriorActivatableWidget* InWidget);

	/**
	 * Removes all widgets registered by this enemy from the viewport and clears
	 * the internal tracking array. Safe to call when no widgets are registered.
	 * Typically called from the death ability or on EndPlay to prevent
	 * orphaned overhead UI elements after the enemy is destroyed.
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveEnemyDrawnWidgetsIfAny();

private:
	/**
	 * Tracks all widgets created and drawn by this enemy instance.
	 * Used to clean up overhead UI elements when the enemy dies or despawns.
	 * UPROPERTY is required to prevent the GC from collecting widgets that are
	 * still referenced here but have no other strong UObject reference path.
	 */
	UPROPERTY()
	TArray<TObjectPtr<UWarriorActivatableWidget>> EnemyDrawnWidgets;
};
