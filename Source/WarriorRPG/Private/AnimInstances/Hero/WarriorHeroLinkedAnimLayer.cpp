// WarriorHeroLinkedAnimLayer.cpp
// Provides access to the main AnimInstance from within a linked anim layer.
//
// A Linked Anim Layer runs as a separate UAnimInstance in the context of the
// SkeletalMeshComponent, but it is NOT the component's primary AnimInstance.
// GetOwningComponent()->GetAnimInstance() navigates back to the primary instance,
// which is where UWarriorHeroAnimInstance lives and owns all character state.

#include "AnimInstances/Hero/WarriorHeroLinkedAnimLayer.h"
#include "AnimInstances/Hero/WarriorHeroAnimInstance.h"

UWarriorHeroAnimInstance* UWarriorHeroLinkedAnimLayer::GetHeroAnimInstance() const
{
    // GetOwningComponent() returns the SkeletalMeshComponent this layer is linked to.
    // This must never be null — a linked layer without an owning component is invalid.
    check(GetOwningComponent());

    // GetAnimInstance() returns the PRIMARY AnimInstance of the SkeletalMeshComponent.
    // For the hero, this is always UWarriorHeroAnimInstance.
    // CastChecked instead of Cast: if this returns null, the layer was linked to a
    // character whose AnimBP does not derive from UWarriorHeroAnimInstance.
    // That is always a Blueprint setup error — crashing in development catches it early.
    return CastChecked<UWarriorHeroAnimInstance>(GetOwningComponent()->GetAnimInstance());
}
