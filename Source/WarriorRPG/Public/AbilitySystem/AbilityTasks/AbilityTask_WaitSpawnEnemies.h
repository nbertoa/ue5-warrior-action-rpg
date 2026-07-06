// AbilityTask_WaitSpawnEnemies.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitSpawnEnemies.generated.h"

class AWarriorEnemyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitSpawnEnemiesDelegate,
                                            const TArray<AWarriorEnemyCharacter*>&,
                                            SpawnedEnemies);

/**
 * UAbilityTask_WaitSpawnEnemies
 *
 * Ability task that listens for a Gameplay Event and asynchronously spawns
 * a configurable number of enemy characters at random reachable locations
 * within a radius around a given origin point.
 *
 * Flow:
 *   1. Activate() registers a delegate on the ASC for the given EventTag.
 *   2. When the event fires, OnGameplayEventReceived() triggers an async
 *      load of SoftEnemyClassToSpawn via the Asset Manager.
 *   3. Once loaded, OnEnemyClassLoaded() spawns up to NumToSpawn enemies
 *      at NavMesh-reachable points and broadcasts OnSpawnFinished.
 *   4. If the class is null or no enemies spawn successfully, DidNotSpawn
 *      is broadcast instead.
 *
 * @note Designed for use in Blueprint ability graphs via the
 *       "Wait Gameplay Event And Spawn Enemies" node.
 */
UCLASS()
class WARRIORRPG_API UAbilityTask_WaitSpawnEnemies : public UAbilityTask
{
	GENERATED_BODY()

public:
	/**
	 * Creates and initializes the task. Call this from a Blueprint ability graph
	 * or from C++ via NewAbilityTask to set up the spawn parameters.
	 *
	 * @param OwningAbility          The ability that owns this task.
	 * @param EventTag               The Gameplay Event tag that triggers the spawn.
	 * @param SoftEnemyClassToSpawn  Soft reference to the enemy class to spawn — loaded asynchronously.
	 * @param NumToSpawn             How many enemies to spawn when the event fires.
	 * @param SpawnOrigin            World-space center point around which enemies are scattered.
	 * @param RandomSpawnRadius      Radius in cm within which spawn locations are randomized.
	 * @return                       The initialized task, ready to be activated by the ability system.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|AbilityTasks",
		meta = (DisplayName = "Wait Gameplay Event And Spawn Enemies", HidePin = "OwningAbility", DefaultToSelf =
			"OwningAbility", BlueprintInternalUseOnly = "true", NumToSpawn = "1", RandomSpawnRadius = "200"))
	static UAbilityTask_WaitSpawnEnemies* WaitSpawnEnemies(UGameplayAbility* OwningAbility,
	                                                       FGameplayTag EventTag,
	                                                       TSoftClassPtr<AWarriorEnemyCharacter> SoftEnemyClassToSpawn,
	                                                       int32 NumToSpawn,
	                                                       const FVector& SpawnOrigin,
	                                                       float RandomSpawnRadius);

	/** Broadcast when at least one enemy was successfully spawned.
	 *  Receives the array of all spawned enemy instances. */
	UPROPERTY(BlueprintAssignable)
	FWaitSpawnEnemiesDelegate OnSpawnFinished;

	/** Broadcast when no enemies could be spawned — either the class failed
	 *  to load, the world was invalid, or every individual SpawnActor call failed. */
	UPROPERTY(BlueprintAssignable)
	FWaitSpawnEnemiesDelegate DidNotSpawn;

	//~ Begin UGameplayTask Interface
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	//~ End UGameplayTask Interface

private:
	FGameplayTag CachedEventTag;
	TSoftClassPtr<AWarriorEnemyCharacter> CachedSoftEnemyClassToSpawn;
	int32 CachedNumToSpawn;
	FVector CachedSpawnOrigin;
	float CachedRandomSpawnRadius;

	/** Handle used to unregister the event delegate in OnDestroy,
	 *  preventing callbacks from firing after the task has ended. */
	FDelegateHandle DelegateHandle;

	/** Callback registered on the ASC for CachedEventTag.
	 *  Triggers the async load of the enemy class when the event fires. */
	void OnGameplayEventReceived(const FGameplayEventData* InPayload);

	/** Callback invoked by the Streamable Manager when SoftEnemyClassToSpawn
	 *  has finished loading. Performs the actual SpawnActor loop. */
	void OnEnemyClassLoaded();
};
