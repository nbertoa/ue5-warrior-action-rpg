// BTService_OrientToTargetActor.h
// Behavior Tree Service that rotates the owning AI pawn to face a target actor
// stored in the Blackboard. Runs every tick while the node is active, smoothly
// interpolating the pawn's rotation toward the target using RInterpTo.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_OrientToTargetActor.generated.h"

/**
 * UBTService_OrientToTargetActor
 *
 * Behavior Tree Service that continuously orients the AI pawn toward a target
 * actor read from the Blackboard. Designed to run alongside attack or strafe
 * behavior so the enemy always faces its target without manual rotation logic
 * in the character or animation blueprint.
 *
 * Configure InTargetActorKey to point to the Blackboard key holding the target,
 * and RotationInterpSpeed to control how quickly the pawn turns.
 */
UCLASS()
class WARRIORRPG_API UBTService_OrientToTargetActor : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_OrientToTargetActor();

    //~ Begin UBTNode Interface
    /**
     * Resolves the Blackboard key selector against the behavior tree's Blackboard asset.
     * Called once when the behavior tree asset is loaded — must be called before
     * TickNode attempts to read from the Blackboard.
     *
     * @param Asset     The behavior tree asset this service belongs to.
     */
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

    /**
     * Returns a human-readable description of this service shown in the BT editor.
     * Includes the target actor key name for quick identification in complex trees.
     *
     * @return  A formatted string describing the service's configuration.
     */
    virtual FString GetStaticDescription() const override;
    //~ End UBTNode Interface

protected:
    /**
     * Called every tick interval while this service's node is active.
     * Reads the target actor from the Blackboard and smoothly rotates the
     * owning pawn toward it using RInterpTo at RotationInterpSpeed.
     *
     * @param OwnerComp     The behavior tree component running this service.
     * @param NodeMemory    Per-instance memory for this node (unused).
     * @param DeltaSeconds  Time elapsed since the last tick.
     */
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
                          uint8* NodeMemory,
                          float DeltaSeconds) override;

private:
    /**
     * Blackboard key that holds the target actor to orient toward.
     * Filtered to AActor-derived objects — configure this in the BT editor.
     */
    UPROPERTY(EditAnywhere,
        Category = "Target")
    FBlackboardKeySelector InTargetActorKey;

    /**
     * Interpolation speed for rotation toward the target actor (degrees per second scale).
     * Higher values produce snappier rotation; lower values produce smoother, slower turns.
     * Tune per enemy type in the behavior tree asset defaults.
     */
    UPROPERTY(EditAnywhere,
        Category = "Target")
    float RotationInterpSpeed = 5.0f;
};
