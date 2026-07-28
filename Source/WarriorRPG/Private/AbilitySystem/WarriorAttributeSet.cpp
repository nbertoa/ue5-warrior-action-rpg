// WarriorAttributeSet.cpp
// Attribute initialization and post-execution callbacks for UWarriorAttributeSet.
// PostGameplayEffectExecute handles clamping for persistent attributes,
// broadcasts normalized values to UI component delegates when available, and
// consumes the DamageTaken intermediary attribute to apply damage to CurrentHealth.

#include "AbilitySystem/WarriorAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Components/UI/HeroUIComponent.h"
#include "Components/UI/PawnUIComponent.h"
#include "Interfaces/PawnUIInterface.h"
#include "Utils/WarriorRPGDebugHelper.h"
#include "Utils/WarriorRPGLogCategories.h"
#include "Utils/WarriorRPGTags.h"
#include "WarriorFunctionLibrary.h"

namespace
{
    float GetNormalizedAttributeValue(float CurrentValue,
                                      float MaxValue)
    {
        const float SafeMaxValue = FMath::Max(MaxValue,
                                              0.0f);
        return SafeMaxValue > 0.0f ? FMath::Clamp(CurrentValue / SafeMaxValue,
                                                  0.0f,
                                                  1.0f) : 0.0f;
    }
}

UWarriorAttributeSet::UWarriorAttributeSet()
{
    // All persistent attributes start at 1.0f as a safe non-zero baseline.
    // Real values are applied by a GE_Initialize Gameplay Effect at possession time.
    InitCurrentHealth(1.0f);
    InitMaxHealth(1.0f);
    InitCurrentRage(1.0f);
    InitMaxRage(1.0f);
    InitAttackPower(1.0f);
    InitDefensePower(1.0f);

    // DamageTaken is a transient message carrier, not a persistent stat.
    // It is written by UGEExecCalc_DamageTaken and consumed immediately in
    // PostGameplayEffectExecute — it should never hold a non-zero value at rest.
    InitDamageTaken(0.0f);
}

void UWarriorAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
    // GetAvatarActor() can return null if the ASC has not yet initialized its
    // actor info, or if the owning actor was already destroyed. Guard before
    // accessing any interface or component on the avatar.
    AActor* AvatarActor = Data.Target.GetAvatarActor();
    if (!ensureMsgf(AvatarActor,
                    TEXT("UWarriorAttributeSet::PostGameplayEffectExecute — " "GetAvatarActor() returned null on the target ASC.")))
    {
        return;
    }

    // Cache the UI interface on first access — avoids a repeated interface cast
    // on every GE execution for the lifetime of this attribute set.
    if (!CachedPawnUIInterface.IsValid())
    {
        CachedPawnUIInterface = TWeakInterfacePtr<IPawnUIInterface>(AvatarActor);
    }

    // UI is optional for base characters such as NPCs. Attribute clamping,
    // damage consumption, and gameplay tags must continue to work without it.
    // A missing interface or component suppresses UI notifications only.
    UPawnUIComponent* PawnUIComponent = CachedPawnUIInterface.IsValid()
        ? CachedPawnUIInterface->GetPawnUIComponent()
        : nullptr;

    // Hero and enemy characters provide a component; NPCs may intentionally not.
    // MaxHealth and CurrentHealth are handled separately because an initialization GE
    // commonly modifies both attributes. Broadcasting unconditionally from both executions
    // would notify the UI twice while the enemy is being initialized.
    if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        // MaxHealth must never be negative. A negative maximum would make the valid
        // CurrentHealth range impossible and would invalidate percentage calculations.
        const float SafeMaxHealth = FMath::Max(GetMaxHealth(),
                                               0.0f);
        SetMaxHealth(SafeMaxHealth);

        // A lower MaxHealth may leave CurrentHealth outside its new valid range.
        // Preserve the current value when possible and clamp only when required.
        const float OldCurrentHealth = GetCurrentHealth();
        const float NewCurrentHealth = FMath::Clamp(OldCurrentHealth,
                                                    0.0f,
                                                    SafeMaxHealth);

        // Notify listeners only when the MaxHealth change also forced CurrentHealth
        // to change. This suppresses the redundant initialization broadcast while
        // still keeping the UI synchronized when health is clamped by a lower maximum.
        if (!FMath::IsNearlyEqual(OldCurrentHealth,
                                  NewCurrentHealth))
        {
            SetCurrentHealth(NewCurrentHealth);

            if (PawnUIComponent)
            {
                // Normalize to [0, 1] before broadcasting — widgets should never know raw values.
                PawnUIComponent->OnCurrentHealthChanged.Broadcast(GetNormalizedAttributeValue(NewCurrentHealth,
                                                                                              SafeMaxHealth));
            }
        }
    }
    else if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
    {
        // Direct CurrentHealth modifiers include initialization and healing effects.
        // Clamp their result before exposing the normalized value to UI listeners.
        const float NewCurrentHealth = FMath::Clamp(GetCurrentHealth(),
                                                    0.0f,
                                                    GetMaxHealth());

        SetCurrentHealth(NewCurrentHealth);

        if (PawnUIComponent)
        {
            // Normalize to [0, 1] before broadcasting — widgets should never know raw values.
            PawnUIComponent->OnCurrentHealthChanged.Broadcast(GetNormalizedAttributeValue(NewCurrentHealth,
                                                                                          GetMaxHealth()));
        }
    }

    // Clamp CurrentRage after any direct modification, same rationale as CurrentHealth above.
    if (Data.EvaluatedData.Attribute == GetCurrentRageAttribute() || Data.EvaluatedData.Attribute == GetMaxRageAttribute())
    {
        const float SafeMaxRage = FMath::Max(GetMaxRage(),
                                             0.0f);
        SetMaxRage(SafeMaxRage);

        const float NewCurrentRage = FMath::Clamp(GetCurrentRage(),
                                                  0.0f,
                                                  SafeMaxRage);

        SetCurrentRage(NewCurrentRage);

        // Rage is hero-exclusive — only broadcast if the avatar has a HeroUIComponent.
        // Enemy characters return nullptr from GetHeroUIComponent() by design.
        if (CachedPawnUIInterface.IsValid())
        {
            if (UHeroUIComponent* HeroUIComponent = CachedPawnUIInterface->GetHeroUIComponent())
            {
                HeroUIComponent->OnCurrentRageChanged.Broadcast(GetNormalizedAttributeValue(NewCurrentRage,
                                                                                            SafeMaxRage));
            }
        }
    }

    if (Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
    {
        const float OldHealth = GetCurrentHealth();
        const float DamageDone = GetDamageTaken();
        const float SafeMaxHealth = FMath::Max(GetMaxHealth(),
                                               0.0f);

        // Reset immediately — DamageTaken must be zero between GE applications.
        // Leaving a stale value would corrupt the next ExecCalc capture for this attribute.
        SetDamageTaken(0.0f);

        const float NewCurrentHealth = FMath::Clamp(OldHealth - DamageDone,
                                                    0.0f,
                                                    SafeMaxHealth);

        SetCurrentHealth(NewCurrentHealth);

        if (PawnUIComponent)
        {
            // Normalize to [0, 1] before broadcasting — same pattern as the direct health block above.
            PawnUIComponent->OnCurrentHealthChanged.Broadcast(GetNormalizedAttributeValue(NewCurrentHealth,
                                                                                          SafeMaxHealth));
        }

        // FMath::IsNearlyZero instead of == 0.0f: floating point arithmetic can produce
        // values like -0.000001f after clamping, which would cause == 0.0f to never fire.
        if (FMath::IsNearlyZero(NewCurrentHealth))
        {
            UWarriorFunctionLibrary::AddGameplayTagToActor(AvatarActor,
                                                           WarriorRPGTags::Shared::Status::Dead);
        }
    }

    if (Data.EvaluatedData.Attribute == GetCurrentRageAttribute() || Data.EvaluatedData.Attribute == GetMaxRageAttribute())
    {
        // Full and None represent mutually exclusive resource-boundary states.
        // Evaluate the empty state first: when MaxRage is zero, CurrentRage is also
        // clamped to zero and must be represented as None rather than Full.
        const bool bIsRageEmpty = FMath::IsNearlyZero(GetCurrentRage());
        const bool bIsRageFull = !bIsRageEmpty && FMath::IsNearlyEqual(GetCurrentRage(),
                                                                        GetMaxRage());

        if (bIsRageFull)
        {
            UWarriorFunctionLibrary::AddGameplayTagToActor(AvatarActor,
                                                           WarriorRPGTags::Player::Status::Rage::Full);
            UWarriorFunctionLibrary::RemoveGameplayTagFromActor(AvatarActor,
                                                                WarriorRPGTags::Player::Status::Rage::None);
        }
        else if (bIsRageEmpty)
        {
            UWarriorFunctionLibrary::RemoveGameplayTagFromActor(AvatarActor,
                                                                WarriorRPGTags::Player::Status::Rage::Full);
            UWarriorFunctionLibrary::AddGameplayTagToActor(AvatarActor,
                                                           WarriorRPGTags::Player::Status::Rage::None);
        }
        else
        {
            UWarriorFunctionLibrary::RemoveGameplayTagFromActor(AvatarActor,
                                                                WarriorRPGTags::Player::Status::Rage::Full);
            UWarriorFunctionLibrary::RemoveGameplayTagFromActor(AvatarActor,
                                                                WarriorRPGTags::Player::Status::Rage::None);
        }
    }
}
