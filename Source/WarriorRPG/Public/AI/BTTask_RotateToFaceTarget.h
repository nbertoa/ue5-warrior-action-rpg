// BTTask_RotateToFaceTarget.h
// Behavior Tree Task that smoothly rotates the owning AI pawn to face a target actor
// stored in the Blackboard. Runs over multiple ticks using per-instance node memory
// and completes when the angular difference falls within AnglePrecision degrees.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RotateToFaceTarget.generated.h"

/**
 * Per-instance memory for UBTTask_RotateToFaceTarget.
 * Stores weak references to the owning pawn and target actor so the task
 * can access them across ticks without holding strong references that
 * prevent GC from collecting destroyed objects.
 *
 * Allocated and managed by the Behavior Tree component via GetInstanceMemorySize().
 */
struct FRotateToFaceTargetTaskMemory
{
	/** Weak reference to the AI pawn performing the rotation. */
	TWeakObjectPtr<APawn> OwningPawn;

	/** Weak reference to the actor the pawn should face. */
	TWeakObjectPtr<AActor> TargetActor;

	/**
	 * Returns true if both the owning pawn and target actor are still valid.
	 * Must be checked at the start of every tick before dereferencing either pointer.
	 */
	bool IsValid() const
	{
		return OwningPawn.IsValid() && TargetActor.IsValid();
	}

	/**
	 * Clears both weak references when the task completes or fails.
	 * Prevents stale pointers from being held after the task ends.
	 */
	void Reset()
	{
		OwningPawn.Reset();
		TargetActor.Reset();
	}
};

/**
 * UBTTask_RotateToFaceTarget
 *
 * Behavior Tree Task that rotates the AI pawn toward a Blackboard-specified
 * target actor using RInterpTo over multiple ticks.
 * Succeeds when the angle between the pawn's forward vector and the direction
 * to the target falls within AnglePrecision degrees.
 * Fails if either the pawn or target actor becomes invalid during execution.
 */
UCLASS()
class WARRIORRPG_API UBTTask_RotateToFaceTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RotateToFaceTarget();

	//~ Begin UBTNode Interface
	/**
	 * Resolves the Blackboard key selector against the behavior tree's Blackboard asset.
	 * Called once when the behavior tree asset is loaded.
	 *
	 * @param Asset     The behavior tree asset this task belongs to.
	 */
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	/**
	 * Returns the size in bytes of FRotateToFaceTargetTaskMemory for per-instance allocation.
	 *
	 * @return  sizeof(FRotateToFaceTargetTaskMemory).
	 */
	virtual uint16 GetInstanceMemorySize() const override;

	/**
	 * Returns a human-readable description shown in the BT editor node.
	 * Includes the target key name and angle precision for quick identification.
	 *
	 * @return  A formatted string describing the task's configuration.
	 */
	virtual FString GetStaticDescription() const override;
	//~ End UBTNode Interface

protected:
	/**
	 * Initializes per-instance memory with the owning pawn and target actor.
	 * Returns Succeeded immediately if already within AnglePrecision,
	 * InProgress otherwise to begin tick-based interpolation.
	 *
	 * @param OwnerComp     The behavior tree component running this task.
	 * @param NodeMemory    Per-instance memory cast to FRotateToFaceTargetTaskMemory.
	 * @return              Succeeded if already facing target, InProgress to continue ticking,
	 *                      Failed if target actor is invalid or not an AActor.
	 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	                                        uint8* NodeMemory) override;

	/**
	 * Called every tick while this task is InProgress.
	 * Interpolates the pawn's rotation toward the target and finishes
	 * with Succeeded when AnglePrecision is reached, or Failed if memory becomes invalid.
	 *
	 * @param OwnerComp     The behavior tree component running this task.
	 * @param NodeMemory    Per-instance memory cast to FRotateToFaceTargetTaskMemory.
	 * @param DeltaSeconds  Time elapsed since the last tick.
	 */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,
	                      uint8* NodeMemory,
	                      float DeltaSeconds) override;

private:
	/**
	 * Returns true if the angle between the pawn's forward vector and the direction
	 * to the target is within AnglePrecision degrees.
	 * Used by both ExecuteTask (early-out) and TickTask (completion check).
	 *
	 * @param QueryPawn     The pawn performing the check. Must not be null.
	 * @param TargetActor   The actor being faced. Must not be null.
	 * @return              True if the angular difference is <= AnglePrecision.
	 */
	bool HasReachedAnglePrecision(APawn* QueryPawn,
	                              AActor* TargetActor) const;

	/**
	 * Maximum angular difference in degrees between the pawn's forward vector
	 * and the direction to the target for the task to be considered complete.
	 * Smaller values require more precise alignment; larger values complete sooner.
	 */
	UPROPERTY(EditAnywhere,
		Category = "Face Target")
	float AnglePrecision = 10.0f;

	/**
	 * Interpolation speed for rotation toward the target (scale factor for RInterpTo).
	 * Higher values produce snappier rotation; lower values produce smoother, slower turns.
	 */
	UPROPERTY(EditAnywhere,
		Category = "Face Target")
	float RotationInterpSpeed = 5.0f;

	/**
	 * Blackboard key that holds the target actor to face.
	 * Filtered to AActor-derived objects — configure this in the BT editor.
	 */
	UPROPERTY(EditAnywhere,
		Category = "Face Target")
	FBlackboardKeySelector InTargetToFaceKey;
};
