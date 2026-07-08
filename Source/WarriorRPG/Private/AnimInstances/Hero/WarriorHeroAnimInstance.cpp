// WarriorHeroAnimInstance.cpp
// Implements hero-specific animation state: relax idle transition
// and future hero-only animation features.

#include "AnimInstances/Hero/WarriorHeroAnimInstance.h"

#include "Characters/WarriorHeroCharacter.h"
#include "Utils/WarriorRPGLogCategories.h"

void UWarriorHeroAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // OwningCharacter is already cached by the parent class (UWarriorCharacterAnimInstance).
    // We downcast it to AWarriorHeroCharacter to access hero-specific properties later.
    if (OwningCharacter)
    {
        OwningHeroCharacter = Cast<AWarriorHeroCharacter>(OwningCharacter);

        if (!OwningHeroCharacter)
        {
            UE_LOG(LogWarriorRPG,
                   Warning,
                   TEXT("UWarriorHeroAnimInstance::NativeInitializeAnimation — " "OwningCharacter is valid but is not an AWarriorHeroCharacter. " "This AnimInstance should only be used on hero character Blueprints."));
        }
    }
}

void UWarriorHeroAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

    if (!OwningHeroCharacter)
    {
        return;
    }

    if (bHasAcceleration)
    {
        // Player is actively moving — reset the idle timer immediately.
        // This ensures the relax state exits the moment input is detected,
        // so the transition back to alert idle feels responsive.
        IdleElapsedTime = 0.0f;
        bShouldEnterRelaxState = false;
    }
    else
    {
        // No movement input — accumulate idle time.
        // Once the threshold is exceeded, the AnimGraph can transition to a relaxed pose
        // (leaning, looking around, shifting weight) to add life to the character.
        IdleElapsedTime += DeltaSeconds;
        bShouldEnterRelaxState = IdleElapsedTime >= EnterRelaxStateThreshold;
    }
}
