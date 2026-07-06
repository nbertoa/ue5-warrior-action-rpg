// HeroGameplayAbility_TargetLock.h
// Gameplay ability that locks the hero's camera and locomotion onto a nearby enemy.
// Activated and cancelled by a single toggle input (Input::Toggleable::TargetLock).
// Uses a box trace to find candidates, selects the nearest, draws a lock widget,
// and updates the widget position every frame via UAbilityTask_ExecuteTaskOnTick.

#pragma once

#include "CoreMinimal.h"
#include "WarriorHeroGameplayAbility.h"
#include "HeroGameplayAbility_TargetLock.generated.h"

class UInputMappingContext;
class UWarriorActivatableWidget;

/**
 * Hero-exclusive target lock ability.
 *
 * On activation, performs a box trace in front of the hero to collect candidate actors,
 * selects the nearest one as the lock target, and draws a screen-space widget over it.
 * The widget position is updated every frame via UAbilityTask_ExecuteTaskOnTick.
 * If no valid target is found, the ability cancels itself immediately.
 *
 * On end (cancelled or completed), movement speed and input mapping context are
 * restored before all runtime state is cleaned up via CleanUp().
 */
UCLASS()
class WARRIORRPG_API UHeroGameplayAbility_TargetLock : public UWarriorHeroGameplayAbility
{
	GENERATED_BODY()

protected:
	//~ Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
	                        const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo,
	                        bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	//~ End UGameplayAbility Interface

	/**
	 * Per-frame tick handler bound to UAbilityTask_ExecuteTaskOnTick::OnAbilityTaskTick.
	 * Each frame: checks if the target or hero died (cancels if so), updates the lock
	 * widget position, and interpolates the hero's rotation toward the locked target
	 * unless a blocking state (Rolling, Blocking) is active.
	 *
	 * @param DeltaTime   Time in seconds since the last frame.
	 */
	UFUNCTION(BlueprintCallable)
	void OnTargetLockTick(float DeltaTime);

private:
	/**
	 * Returns true if either the current locked target or the hero has the
	 * Shared::Status::Dead tag on their ASC.
	 * Used by OnTargetLockTick to detect when the lock session should end.
	 * Guards internally against a null CurrentLockedActor.
	 */
	bool IsTargetActorOrHeroDead();

	/**
	 * Switches the current lock target to the nearest enemy in the direction
	 * specified by InSwitchDirectionTag (Player::Event::SwitchTarget::Left or Right).
	 * Re-runs the box trace to refresh AvailableActorsToLock, classifies actors
	 * into left/right buckets via GetAvailableActorsAroundTarget, then selects
	 * the nearest from the appropriate bucket.
	 *
	 * @param InSwitchDirectionTag   Player::Event::SwitchTarget::Left or ::Right.
	 */
	UFUNCTION(BlueprintCallable)
	void SwitchTarget(const FGameplayTag& InSwitchDirectionTag);

	/**
	 * Entry point for the lock sequence.
	 * Populates AvailableActorsToLock, selects the nearest target, draws the widget,
	 * and positions it. Cancels the ability if no valid target is found.
	 */
	void TryLockOnTarget();

	/**
	 * Performs a box trace in front of the hero and fills AvailableActorsToLock
	 * with all hit actors that are not the hero itself.
	 * Uses BoxTraceDistance, TraceBoxSize, and BoxTraceChannel as parameters.
	 * Empties AvailableActorsToLock before tracing to ensure a fresh result set.
	 */
	void UpdateAvailableActorsToLock();

	/**
	 * Returns the actor in InAvailableActors closest to the hero's world location.
	 * Delegates to UGameplayStatics::FindNearestActor.
	 *
	 * @param InAvailableActors   Candidate actors to evaluate. Safe to call with an empty array.
	 * @return                    The nearest actor, or nullptr if the array is empty.
	 */
	AActor* GetNearestTargetFromAvailableActors(const TArray<AActor*>& InAvailableActors);

	/**
	 * Classifies all actors in AvailableActorsToLock (excluding CurrentLockedActor)
	 * into left and right buckets relative to the vector from the hero to the current target.
	 * Uses cross product Z sign to determine side — positive Z means right, negative means left.
	 * Cancels the ability if CurrentLockedActor is null or AvailableActorsToLock is empty.
	 *
	 * @param OutActorsOnLeft    Receives all actors to the left of the current target direction.
	 * @param OutActorsOnRight   Receives all actors to the right of the current target direction.
	 */
	void GetAvailableActorsAroundTarget(TArray<AActor*>& OutActorsOnLeft,
	                                    TArray<AActor*>& OutActorsOnRight);

	/**
	 * Creates and adds the target lock widget to the viewport if not already drawn.
	 * Crashes in development if TargetLockWidgetClass has not been assigned in Blueprint.
	 */
	void DrawTargetLockWidget();

	/**
	 * Projects CurrentLockedActor's world location to screen space and positions
	 * DrawnTargetLockWidget centered over it.
	 * Queries the widget's SizeBox on the first call to cache TargetLockWidgetSize;
	 * subsequent calls reuse the cached value.
	 * Cancels the ability if either DrawnTargetLockWidget or CurrentLockedActor is null.
	 */
	void SetTargetLockWidgetPosition();

	/**
	 * Caches the hero's current MaxWalkSpeed and sets it to TargetLockMaxWalkSpeed.
	 * Called from ActivateAbility to slow the hero during target lock.
	 * The cached value is restored by ResetTargetLockMovement in EndAbility.
	 */
	void InitTargetLockMovement();

	/**
	 * Adds TargetLockMappingContext at priority 3 to override the default look bindings
	 * with target-switch-aware input during the lock session.
	 * Called from ActivateAbility after TryLockOnTarget succeeds.
	 */
	void InitTargetLockMappingContext();

	/**
	 * Cancels this ability through the standard GAS cancellation path.
	 * Called when no valid lock target is found or when widget state becomes invalid.
	 */
	void CancelTargetLockAbility();

	/**
	 * Resets all runtime state accumulated during the lock session.
	 * Empties AvailableActorsToLock, nulls CurrentLockedActor,
	 * removes DrawnTargetLockWidget from the viewport and nulls it,
	 * resets TargetLockWidgetSize and CachedDefaultMaxWalkSpeed.
	 * Always called from EndAbility before Super::EndAbility.
	 */
	void CleanUp();

	/**
	 * Restores the hero's MaxWalkSpeed to CachedDefaultMaxWalkSpeed.
	 * No-op if CachedDefaultMaxWalkSpeed is zero (ability ended before Init completed).
	 * Called from EndAbility before CleanUp.
	 */
	void ResetTargetLockMovement();

	/**
	 * Removes TargetLockMappingContext from the Enhanced Input subsystem.
	 * Guards against a null hero controller — safe to call even if the controller
	 * was invalidated before EndAbility fires.
	 * Called from EndAbility before CleanUp.
	 */
	void ResetTargetLockMappingContext();

	/**
	 * Interpolates the hero's controller rotation and actor yaw toward CurrentLockedActor
	 * each frame while the target lock is active.
	 * Applies TargetLockCameraOffsetDistance as a negative pitch offset to keep the
	 * target visible in the lower portion of the frame.
	 * Roll is zeroed on both the controller and the character to prevent banking artifacts.
	 * Called from OnTargetLockTick only when bShouldOverrideRotation is true.
	 *
	 * @param DeltaTime   Time in seconds since the last frame. Passed to RInterpTo.
	 */
	void OverrideRotation(float DeltaTime);

	// ─── Designer-tunable parameters ────────────────────────────────────────

	/** Half-length of the box trace along the hero's forward vector, in cm. */
	UPROPERTY(EditDefaultsOnly,
		Category = "Settings")
	float BoxTraceDistance = 5000.0f;

	/**
	 * Full size of the box trace volume in cm.
	 * Divided by 2 internally before passing to BoxTraceMultiForObjects.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Settings")
	FVector TraceBoxSize = FVector(5000.0f,
	                               5000.0f,
	                               300.0f);

	/** Object channels included in the box trace. Configure in Blueprint defaults. */
	UPROPERTY(EditDefaultsOnly,
		Category = "Settings")
	TArray<TEnumAsByte<EObjectTypeQuery>> BoxTraceChannel;

	/**
	 * Widget class to instantiate and display over the locked target.
	 * Must be assigned in Blueprint defaults — crashes in development if null.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Settings")
	TSubclassOf<UWarriorActivatableWidget> TargetLockWidgetClass;

	/** Interpolation speed for rotating the hero and controller toward the locked target.
	 *  Higher values snap faster; lower values produce a smoother tracking feel. */
	UPROPERTY(EditDefaultsOnly,
		Category = "Settings")
	float TargetLockRotationInterpSpeed = 5.0f;

	/**
	 * MaxWalkSpeed applied to the hero while the target lock is active.
	 * Lower than the default walk speed to enforce deliberate, controlled movement
	 * during a lock session. Restored to the cached default on EndAbility.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Settings")
	float TargetLockMaxWalkSpeed = 150.0f;

	/**
	 * Input Mapping Context added at priority 3 during the lock session.
	 * Overrides the default look bindings to route the right stick / mouse
	 * to the target switch input instead of free camera look.
	 * Must be assigned in Blueprint defaults.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Settings")
	TObjectPtr<UInputMappingContext> TargetLockMappingContext = nullptr;

	/**
	 * Negative pitch offset applied to the LookAt rotation in OverrideRotation.
	 * Tilts the camera slightly downward so the locked target appears in the
	 * lower portion of the frame rather than dead center.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Settings")
	float TargetLockCameraOffsetDistance = 20.0f;

	// ─── Runtime state ───────────────────────────────────────────────────────

	/** Actors collected by the box trace during TryLockOnTarget. Cleared in CleanUp. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AvailableActorsToLock;

	/** The actor currently selected as the lock target. Null when no target is locked. */
	UPROPERTY()
	TObjectPtr<AActor> CurrentLockedActor = nullptr;

	/** The widget instance drawn over CurrentLockedActor. Null when not active. */
	UPROPERTY()
	TObjectPtr<UWarriorActivatableWidget> DrawnTargetLockWidget = nullptr;

	/**
	 * Cached screen-space size of DrawnTargetLockWidget, in pixels.
	 * Queried once from the widget's SizeBox on the first SetTargetLockWidgetPosition call
	 * and reused on all subsequent calls to avoid repeated tree traversal.
	 */
	UPROPERTY()
	FVector2D TargetLockWidgetSize = FVector2D::ZeroVector;

	/**
	 * The hero's MaxWalkSpeed before the target lock reduced it.
	 * Captured by InitTargetLockMovement and restored by ResetTargetLockMovement.
	 * Reset to 0 in CleanUp — a zero value signals that Init did not complete,
	 * so ResetTargetLockMovement safely skips the restore.
	 */
	UPROPERTY()
	float CachedDefaultMaxWalkSpeed = 0.0f;
};
