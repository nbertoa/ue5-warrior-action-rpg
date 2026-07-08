// WarriorBaseAnimInstance.h
// Root AnimInstance class for all animation instances in WarriorRPG.
// Extend this to add project-wide animation functionality that all
// AnimInstances should share (e.g., gameplay tag access, combat state queries).

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "WarriorBaseAnimInstance.generated.h"

/**
 * Abstract base AnimInstance for the WarriorRPG project.
 * All animation instances should inherit from this class rather than UAnimInstance
 * directly, ensuring consistent access to shared project systems.
 */
UCLASS()
class WARRIORRPG_API UWarriorBaseAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

protected:
    /**
     * Returns true if the owning pawn's ASC has the given gameplay tag.
     * Thread-safe — can be called from the AnimGraph worker thread via
     * NativeThreadSafeUpdateAnimation without risking data tearing.
     * Returns false if the owning pawn is null (e.g., during animation preview).
     *
     * @param TagToCheck    The gameplay tag to query on the owning pawn's ASC.
     * @return              True if the pawn's ASC has a matching tag, false otherwise.
     */
    UFUNCTION(BlueprintPure,
        meta = (BlueprintThreadSafe))
    bool DoesOwnerHaveTag(FGameplayTag TagToCheck) const;
};
