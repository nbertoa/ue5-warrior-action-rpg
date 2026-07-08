// WarriorHeroLinkedAnimLayer.h
// Linked Animation Layer for the hero character.
// Used to modularize the hero's Animation Blueprint by separating animation logic
// into swappable layers — e.g., different locomotion or attack sets per weapon class.
//
// Architecture:
//   The main ABP (ABP_Hero) defines the overall state machine and reserves slots for
//   linked layers. At runtime, a concrete layer (e.g., ABP_Hero_Axe) is linked into
//   those slots via LinkAnimClassLayers(). This layer class provides the accessor
//   needed to read state from the main ABP's UWarriorHeroAnimInstance.

#pragma once

#include "CoreMinimal.h"
#include "AnimInstances/WarriorBaseAnimInstance.h"
#include "WarriorHeroLinkedAnimLayer.generated.h"

class UWarriorHeroAnimInstance;

/**
 * Linked Anim Layer for the WarriorRPG hero.
 * Allows swapping animation sub-graphs at runtime without modifying the main AnimBP.
 *
 * Key design point: this class itself does not own any animation state. All state lives
 * in UWarriorHeroAnimInstance (the main ABP's AnimInstance). This layer reads that state
 * via GetHeroAnimInstance() to drive its own AnimGraph decisions.
 *
 * Typical use case: swapping the UpperBody or FullBody layer when the hero equips a
 * different weapon class, giving each weapon its own locomotion and combat animations.
 */
UCLASS()
class WARRIORRPG_API UWarriorHeroLinkedAnimLayer : public UWarriorBaseAnimInstance
{
    GENERATED_BODY()

public:
    /**
     * Returns the main AnimInstance of the owning SkeletalMeshComponent, cast to
     * UWarriorHeroAnimInstance. Use this in the linked layer's AnimGraph to read
     * locomotion speed, combat state, or any other data computed by the main ABP.
     *
     * Marked BlueprintThreadSafe so it can be called from the AnimGraph's worker thread
     * during NativeThreadSafeUpdateAnimation. The underlying GetAnimInstance() call is
     * safe to make from a non-game thread in this context.
     *
     * @return  The hero's main AnimInstance. CastChecked will crash in development
     *          if the owning character's AnimInstance is not UWarriorHeroAnimInstance —
     *          this indicates the Linked Layer was assigned to the wrong character Blueprint.
     */
    UFUNCTION(BlueprintPure,
        meta = (BlueprintThreadSafe))
    UWarriorHeroAnimInstance* GetHeroAnimInstance() const;
};
