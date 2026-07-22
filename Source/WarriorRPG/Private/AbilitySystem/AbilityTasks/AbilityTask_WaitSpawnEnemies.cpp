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
    if (!ensureMsgf(AbilitySystemComponent.Get(),
                    TEXT("[UAbilityTask_WaitSpawnEnemies] Activate — AbilitySystemComponent is null. " "The task cannot register its event delegate.")))
    {
        EndTask();
        return;
    }

    FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedEventTag);

    DelegateHandle = Delegate.AddUObject(this,
                                         &ThisClass::OnGameplayEventReceived);
}

void UAbilityTask_WaitSpawnEnemies::OnDestroy(bool bInOwnerFinished)
{
    // Always unregister before the task is garbage collected — otherwise the
    // callback can fire on a destroyed UObject if the ASC outlives this task.
    if (AbilitySystemComponent.Get())
    {
        if (FGameplayEventMulticastDelegate* Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.Find(CachedEventTag))
        {
            Delegate->Remove(DelegateHandle);
        }
    }

    Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_WaitSpawnEnemies::OnGameplayEventReceived(const FGameplayEventData* InPayload)
{
    if (bSpawnTriggered)
    {
        return;
    }

    bSpawnTriggered = true;

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
    UWorld* World = GetWorld();

    // World can be null if the level was unloaded between the async load request
    // and this callback (e.g., during a level transition or PIE stop).
    if (!ensureMsgf(World,
                    TEXT("[UAbilityTask_WaitSpawnEnemies] OnEnemyClassLoaded — World is null. " "The level may have been unloaded during the async load.")))
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
        }

        EndTask();
        return;
    }

    // Actor spawning is authoritative in Unreal. A client-side task may still
    // receive the event for prediction, but must never create non-replicated
    // enemies locally.
    if (World->GetNetMode() == NM_Client)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
        }

        EndTask();
        return;
    }

    UClass* LoadedClass = CachedSoftEnemyClassToSpawn.Get();

    // LoadedClass can be null if the asset failed to load or was unloaded
    // between the RequestAsyncLoad call and this callback.
    if (!ensureMsgf(LoadedClass,
                    TEXT("[UAbilityTask_WaitSpawnEnemies] OnEnemyClassLoaded — LoadedClass is null. " "The enemy asset may have failed to load.")))
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
        }

        EndTask();
        return;
    }

    if (!ensureMsgf(AbilitySystemComponent.Get(),
                    TEXT("[UAbilityTask_WaitSpawnEnemies] OnEnemyClassLoaded — AbilitySystemComponent " "is null. The ASC may have been destroyed during the async load.")))
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
        }

        EndTask();
        return;
    }

    AActor* AvatarActor = AbilitySystemComponent->GetAvatarActor();

    if (!ensureMsgf(IsValid(AvatarActor),
                    TEXT("[UAbilityTask_WaitSpawnEnemies] OnEnemyClassLoaded — AvatarActor is null " "or pending kill. Cannot determine spawn facing rotation.")))
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

    // Cache the facing rotation once — all enemies spawned in this batch
    // face the same direction as the summoner at the moment of summoning.
    const FRotator SpawnFacingRotation = AvatarActor->GetActorForwardVector().ToOrientationRotator();

    for (int32 i = 0; i < CachedNumToSpawn; i++)
    {
        FVector RandomLocation = FVector::ZeroVector;
        const bool bFoundReachableLocation = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this,
                                                                                                     CachedSpawnOrigin,
                                                                                                     RandomLocation,
                                                                                                     CachedRandomSpawnRadius);

        if (!bFoundReachableLocation)
        {
            UE_LOG(LogTemp,
                   Warning,
                   TEXT("[UAbilityTask_WaitSpawnEnemies] No reachable NavMesh location found at index %d."),
                   i);
            continue;
        }

        // Offset upward so the enemy spawns above the ground and falls into place,
        // avoiding cases where the capsule starts inside the floor geometry.
        RandomLocation += FVector(0.0f,
                                  0.0f,
                                  150.0f);

        AWarriorEnemyCharacter* SpawnedEnemy = World->SpawnActor<AWarriorEnemyCharacter>(LoadedClass,
                                                                                         RandomLocation,
                                                                                         SpawnFacingRotation,
                                                                                         SpawnParam);

        // SpawnActor can return nullptr in edge cases even with AdjustIfPossibleButAlwaysSpawn:
        // invalid NavMesh location, extreme geometry overlap, or level teardown between the
        // async load and this callback. Skip the failed slot and let the array-empty check
        // below handle the broadcast — a hard check here would crash and leave already-spawned
        // enemies orphaned with EndTask never called.
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
