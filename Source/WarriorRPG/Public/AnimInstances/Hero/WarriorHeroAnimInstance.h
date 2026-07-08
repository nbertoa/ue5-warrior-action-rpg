// WarriorHeroAnimInstance.h
// Hero-specific AnimInstance that extends the character locomotion data
// with player-only animation state (relax idle, future combat state, etc.).

#pragma once

#include "CoreMinimal.h"
#include "AnimInstances/WarriorCharacterAnimInstance.h"
#include "WarriorHeroAnimInstance.generated.h"

class AWarriorHeroCharacter;

/**
 * AnimInstance for the hero character.
 * Inherits locomotion data (GroundSpeed, bHasAcceleration) from the parent class
 * and adds hero-specific animation state such as the relaxed idle transition.
 *
 * The relax state system tracks how long the hero has been idle and triggers
 * a relaxed animation after a configurable threshold, adding life to the character
 * when the player is AFK or reading UI.
 */
UCLASS()
class WARRIORRPG_API UWarriorHeroAnimInstance : public UWarriorCharacterAnimInstance
{
    GENERATED_BODY()

public:
    //~ Begin UAnimInstance Interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
    //~ End UAnimInstance Interface

protected:
    /** Cached reference to the owning character as AWarriorHeroCharacter for hero-specific API access. */
    UPROPERTY(VisibleDefaultsOnly,
        BlueprintReadOnly,
        Category = "AnimData|References")
    TObjectPtr<AWarriorHeroCharacter> OwningHeroCharacter;

    /**
     * Whether the hero has been idle long enough to enter the relaxed animation state.
     * Driven by IdleElapsedTime vs EnterRelaxStateThreshold comparison.
     * Consumed by the AnimGraph to blend into a relaxed idle pose.
     */
    UPROPERTY(VisibleDefaultsOnly,
        BlueprintReadOnly,
        Category = "AnimData|LocomotionData")
    bool bShouldEnterRelaxState;

    /**
     * Time in seconds the hero must remain idle before transitioning to the relaxed state.
     * Exposed as EditDefaultsOnly so designers can tune the feel without recompiling.
     */
    UPROPERTY(EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "AnimData|LocomotionData")
    float EnterRelaxStateThreshold = 5.0f;

private:
    /**
     * Accumulated seconds the hero has been idle (no acceleration input).
     * Not exposed to Blueprint — it's an internal timer that drives bShouldEnterRelaxState.
     * Resets to zero the moment the player provides movement input.
     */
    float IdleElapsedTime = 0.0f;
};
