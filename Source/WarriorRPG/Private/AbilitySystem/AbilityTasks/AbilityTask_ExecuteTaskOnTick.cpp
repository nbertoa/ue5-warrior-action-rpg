// AbilityTask_ExecuteTaskOnTick.cpp
// Implementation of the generic per-frame ability task.

#include "AbilitySystem/AbilityTasks/AbilityTask_ExecuteTaskOnTick.h"

UAbilityTask_ExecuteTaskOnTick::UAbilityTask_ExecuteTaskOnTick()
{
    // Enable per-frame TickTask calls for this task.
    // Disabled by default on UAbilityTask — must be opted in explicitly.
    bTickingTask = true;
}

UAbilityTask_ExecuteTaskOnTick* UAbilityTask_ExecuteTaskOnTick::ExecuteTaskOnTick(UGameplayAbility* OwningAbility)
{
    check(OwningAbility);

    UAbilityTask_ExecuteTaskOnTick* Node = NewAbilityTask<UAbilityTask_ExecuteTaskOnTick>(OwningAbility);

    return Node;
}

void UAbilityTask_ExecuteTaskOnTick::TickTask(float DeltaTime)
{
    Super::TickTask(DeltaTime);

    // ShouldBroadcastAbilityTaskDelegates() verifies that the owning ability is still
    // active and the avatar actor is still valid. If either has been destroyed or the
    // ability has ended while this task was in flight, broadcasting would fire delegates
    // on a dead ability — EndTask() self-destructs the task cleanly instead.
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnAbilityTaskTick.Broadcast(DeltaTime);
    }
    else
    {
        EndTask();
    }
}
