// AbilityTask_WaitSpawnEnemies.cpp

#include "AbilitySystem/AbilityTasks/AbilityTask_WaitSpawnEnemies.h"

#include "AbilitySystemComponent.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "Engine/AssetManager.h"
#include "NavigationSystem.h"

UAbilityTask_WaitSpawnEnemies* UAbilityTask_WaitSpawnEnemies::WaitSpawnEnemies(UGameplayAbility* OwningAbility,
                                                                               FGameplayTag EventTag,
                                                                               TSoftClassPtr<AWarriorEnemyCharacter> SoftEnemyClassToSpawn,
                                                                               int32 NumToSpawn,
                                                                               const FVector& SpawnOrigin,
                                                                               float RandomSpawnRadius)
{
    UAbilityTask_WaitSpawnEnemies* Node = NewAbilityTask<UAbilityTask_WaitSpawnEnemies>(OwningAbility);
    Node->CachedEventTag = EventTag;
    Node->CachedSoftEnemyClassToSpawn = SoftEnemyClassToSpawn;
    Node->CachedNumToSpawn = NumToSpawn;
    Node->CachedSpawnOrigin = SpawnOrigin;
    Node->CachedRandomSpawnRadius = RandomSpawnRadius;

    return Node;
}

void UAbilityTask_WaitSpawnEnemies::Activate()
{
    // Register directly on the ASC's generic event map instead of using
    // UAbilityTask_WaitGameplayEvent, because we need to chain into an async
    // load before spawning — a single WaitGameplayEvent node cannot express that.
    FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedEventTag);

    DelegateHandle = Delegate.AddUObject(this,
                                         &ThisClass::OnGameplayEventReceived);
}

void UAbilityTask_WaitSpawnEnemies::OnDestroy(bool bInOwnerFinished)
{
    // Always unregister before the task is garbage collected — otherwise the
    // callback can fire on a destroyed UObject if the ASC outlives this task.
    FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedEventTag);

    Delegate.Remove(DelegateHandle);

    Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_WaitSpawnEnemies::OnGameplayEventReceived(const FGameplayEventData* InPayload)
{
    if (ensure(!CachedSoftEnemyClassToSpawn.IsNull()))
    {
        // Kick off an async load — the class may not be in memory yet.
        // CreateUObject is safe here because the Streamable Manager holds a strong
        // reference to this task until the callback fires or the handle is released.
        UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(CachedSoftEnemyClassToSpawn.ToSoftObjectPath(),
                                                                     FStreamableDelegate::CreateUObject(this,
                                                                                                        &ThisClass::OnEnemyClassLoaded));
    }
    else
    {
        // Soft class pointer was null — nothing to load, nothing to spawn.
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
        }

        EndTask();
    }
}

void UAbilityTask_WaitSpawnEnemies::OnEnemyClassLoaded()
{
    UClass* LoadedClass = CachedSoftEnemyClassToSpawn.Get();
    UWorld* World = GetWorld();

    // Both checks are required: LoadedClass can be null if the asset failed to load,
    // and World can be null if the level was unloaded between the async request and
    // this callback (e.g., during PIE stop or a level transition).
    if (!LoadedClass || !World)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
        }

        EndTask();
        return;
    }

    TArray<AWarriorEnemyCharacter*> SpawnedEnemies;

    FActorSpawnParameters SpawnParam;
    SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    for (int32 i = 0; i < CachedNumToSpawn; i++)
    {
        FVector RandomLocation;
        UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this,
                                                                CachedSpawnOrigin,
                                                                RandomLocation,
                                                                CachedRandomSpawnRadius);

        // Offset upward so the enemy spawns above the ground and falls into place,
        // avoiding cases where the capsule starts inside the floor geometry.
        RandomLocation += FVector(0.0f,
                                  0.0f,
                                  150.0f);

        const FRotator SpawnFacingRotation = AbilitySystemComponent->GetAvatarActor()->GetActorForwardVector().ToOrientationRotator();

        AWarriorEnemyCharacter* SpawnedEnemy = World->SpawnActor<AWarriorEnemyCharacter>(LoadedClass,
                                                                                         RandomLocation,
                                                                                         SpawnFacingRotation,
                                                                                         SpawnParam);

        // SpawnActor can return nullptr in edge cases even with AdjustIfPossibleButAlwaysSpawn:
        // invalid NavMesh location, extreme geometry overlap, or level teardown between the
        // async load and this callback. A hard check here would crash the game in those
        // scenarios and leave already-spawned enemies orphaned with EndTask never called.
        // Skip the failed slot, log it, and let the array-empty check below handle broadcast.
        if (!ensureMsgf(IsValid(SpawnedEnemy),
                        TEXT("[UAbilityTask_WaitSpawnEnemies] SpawnActor returned null " "for class [%s] at index %d. NavMesh location or collision may be invalid."),
                        *LoadedClass->GetName(),
                        i))
        {
            continue;
        }

        SpawnedEnemies.Add(SpawnedEnemy);
    }

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        if (!SpawnedEnemies.IsEmpty())
        {
            OnSpawnFinished.Broadcast(SpawnedEnemies);
        }
        else
        {
            // Every SpawnActor call failed — treat this as a spawn failure
            // so the ability graph can react (e.g., skip the summon animation).
            DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
        }
    }

    EndTask();
}
