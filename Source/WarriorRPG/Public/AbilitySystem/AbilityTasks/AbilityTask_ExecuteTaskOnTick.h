// AbilityTask_ExecuteTaskOnTick.h
// Generic ability task that exposes per-frame tick to the owning ability via a delegate.
// Use this task instead of enabling tick on the ability directly — it keeps the ability
// event-driven and allows tick to be started and stopped independently of ability lifetime.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_ExecuteTaskOnTick.generated.h"

/**
 * Delegate broadcast every frame while the task is active.
 * Bind to this from the owning ability to drive per-frame logic
 * (e.g., updating a target lock widget position).
 *
 * @param DeltaTime   Time in seconds since the last frame.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityTaskTickDelegate,
                                            float,
                                            DeltaTime);

/**
 * Ability task that ticks every frame and broadcasts OnAbilityTaskTick.
 *
 * Prefer this over enabling tick on UGameplayAbility directly — abilities are
 * event-driven state machines, and per-frame polling should be isolated to a
 * task that can be created and destroyed independently of the ability's lifetime.
 *
 * Usage:
 *   UAbilityTask_ExecuteTaskOnTick* TickTask = UAbilityTask_ExecuteTaskOnTick::ExecuteTaskOnTick(this);
 *   TickTask->OnAbilityTaskTick.AddDynamic(this, &ThisClass::OnTaskTick);
 *   TickTask->ReadyForActivation();
 */
UCLASS()
class WARRIORRPG_API UAbilityTask_ExecuteTaskOnTick : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_ExecuteTaskOnTick();

	/**
	 * Factory method — creates and returns the task bound to the owning ability.
	 * Must be called from within an active UGameplayAbility.
	 * HidePin + DefaultToSelf hide the OwningAbility pin in Blueprint graphs;
	 * BlueprintInternalUseOnly prevents direct use — callers must go through this factory.
	 *
	 * @param OwningAbility   The ability that owns and drives this task. Must not be null.
	 * @return                The created task, ready for ReadyForActivation().
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|AbilityTasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_ExecuteTaskOnTick* ExecuteTaskOnTick(UGameplayAbility* OwningAbility);

	//~ Begin UGameplayTask Interface
	virtual void TickTask(float DeltaTime) override;
	//~ End UGameplayTask Interface

	/**
	 * Broadcast every frame while the task is active and the owning ability is valid.
	 * Bind to this delegate from the owning ability to receive per-frame callbacks.
	 * The task calls EndTask() automatically if the ability becomes invalid mid-tick.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnAbilityTaskTickDelegate OnAbilityTaskTick;
};
