// WarriorBaseAnimInstance.cpp
// Implements shared animation utilities available to all AnimInstances in WarriorRPG.

#include "AnimInstances/WarriorBaseAnimInstance.h"

#include "WarriorFunctionLibrary.h"

bool UWarriorBaseAnimInstance::DoesOwnerHaveTag(FGameplayTag TagToCheck) const
{
    APawn* OwningPawn = TryGetPawnOwner();

    // Return false gracefully if the pawn is null — this can happen during
    // animation preview in the editor where no pawn owns the AnimInstance.
    return OwningPawn ? UWarriorFunctionLibrary::NativeDoesActorHaveTag(OwningPawn,
                                                                        TagToCheck) : false;
}
