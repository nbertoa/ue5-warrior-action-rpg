// WarriorFunctionLibrary.cpp
// Implementation of GAS utility functions and UI widget lookup for WarriorRPG.
// All functions delegate tag operations to UWarriorAbilitySystemComponent,
// retrieved via NativeGetWarriorASCFromActor.

#include "WarriorFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "Interfaces/PawnCombatInterface.h"
#include "Settings/WarriorUISettings.h"
#include "Utils/WarriorRPGLogCategories.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/WarriorRPGTags.h"

UWarriorAbilitySystemComponent* UWarriorFunctionLibrary::NativeGetWarriorASCFromActor(AActor* InActor)
{
    check(InActor);

    // GetAbilitySystemComponent returns null if the actor does not implement
    // IAbilitySystemInterface. Use NullAllowed so the cast does not crash on a null
    // input — the ensureMsgf below provides a clear diagnostic instead.
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor);

    if (!ensureMsgf(ASC,
                    TEXT("UWarriorFunctionLibrary::NativeGetWarriorASCFromActor — " "Actor [%s] does not have an AbilitySystemComponent. " "Ensure it implements IAbilitySystemInterface."),
                    *InActor->GetName()))
    {
        return nullptr;
    }

    // CastChecked with NullAllowed: null was already handled above.
    // If ASC is non-null but not a UWarriorAbilitySystemComponent, that is a
    // project setup error (wrong ASC class) — crash immediately in development.
    return CastChecked<UWarriorAbilitySystemComponent>(ASC,
                                                       ECastCheckedType::NullAllowed);
}

void UWarriorFunctionLibrary::AddGameplayTagToActor(AActor* InActor,
                                                    FGameplayTag InTag)
{
    UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);
    if (!ASC)
    {
        return;
    }

    if (!ASC->HasMatchingGameplayTag(InTag))
    {
        ASC->AddLooseGameplayTag(InTag);
    }
}

void UWarriorFunctionLibrary::RemoveGameplayTagFromActor(AActor* InActor,
                                                         FGameplayTag InTag)
{
    UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);
    if (!ASC)
    {
        return;
    }

    if (ASC->HasMatchingGameplayTag(InTag))
    {
        ASC->RemoveLooseGameplayTag(InTag);
    }
}

bool UWarriorFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor,
                                                     FGameplayTag InTag)
{
    UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);
    if (!ASC)
    {
        return false;
    }

    return ASC->HasMatchingGameplayTag(InTag);
}

void UWarriorFunctionLibrary::BP_DoesActorHaveTag(AActor* InActor,
                                                  FGameplayTag InTag,
                                                  EWarriorConfirmType& OutConfirmType)
{
    // Delegate to the Native variant and convert the bool result to the enum
    // that ExpandEnumAsExecs will turn into execution pins in Blueprint.
    OutConfirmType = NativeDoesActorHaveTag(InActor,
                                            InTag) ? EWarriorConfirmType::Yes : EWarriorConfirmType::No;
}

UPawnCombatComponent* UWarriorFunctionLibrary::NativeGetPawnCombatComponentFromActor(AActor* InActor)
{
    check(InActor);

    IPawnCombatInterface* PawnCombatInterface = Cast<IPawnCombatInterface>(InActor);

    return PawnCombatInterface != nullptr ? PawnCombatInterface->GetPawnCombatComponent() : nullptr;
}

UPawnCombatComponent* UWarriorFunctionLibrary::BP_GetPawnCombatComponentFromActor(AActor* InActor,
                                                                                  EWarriorValidType& OutValidType)
{
    UPawnCombatComponent* CombatComponent = NativeGetPawnCombatComponentFromActor(InActor);

    OutValidType = CombatComponent != nullptr ? EWarriorValidType::Valid : EWarriorValidType::Invalid;

    return CombatComponent;
}

TSubclassOf<UWarriorActivatableWidget> UWarriorFunctionLibrary::GetWidgetClassByTag(FGameplayTag InWidgetTag)
{
    const UWarriorUISettings* WarriorUISettings = GetDefault<UWarriorUISettings>();
    check(WarriorUISettings);

    // A missing entry means the designer forgot to register this widget class
    // in Project Settings → Warrior UI Settings. Crash immediately in development
    // so the omission is caught before it silently produces a null widget at runtime.
    checkf(WarriorUISettings->WidgetClassByTag.Contains(InWidgetTag),
           TEXT("UWarriorFunctionLibrary::GetWidgetClassByTag — " "No widget class registered for tag [%s]. " "Add the entry to Warrior UI Settings in Project Settings."),
           *InWidgetTag.ToString());

    return WarriorUISettings->WidgetClassByTag.FindRef(InWidgetTag);
}

bool UWarriorFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn,
                                                  APawn* TargetPawn)
{
    check(QueryPawn);
    check(TargetPawn);

    // Team affiliation is stored on the controller, not the pawn — the pawn is
    // a physical representation while the controller owns the decision-making identity.
    // Hero controller uses team ID 0; enemy AI controllers use team ID 1.
    // Any mismatch in team IDs means the pawns are on opposing factions.
    AController* QueryPawnController = QueryPawn->GetController();
    check(QueryPawnController);

    AController* TargetPawnController = TargetPawn->GetController();
    check(TargetPawnController);

    // CastChecked: both controllers must implement IGenericTeamAgentInterface —
    // failing here means a controller was not set up with team affiliation,
    // which is always a project configuration error.
    const IGenericTeamAgentInterface* QueryTeamAgent = CastChecked<IGenericTeamAgentInterface>(QueryPawnController);
    const IGenericTeamAgentInterface* TargetTeamAgent = CastChecked<IGenericTeamAgentInterface>(TargetPawnController);

    return QueryTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
}

float UWarriorFunctionLibrary::GetScalableFloatValueAtLevel(const FScalableFloat& InScalableFloat,
                                                            float InLevel)
{
    return InScalableFloat.GetValueAtLevel(InLevel);
}

FGameplayTag UWarriorFunctionLibrary::ComputeHitReactDirectionTag(AActor* InAttacker,
                                                                  AActor* InVictim,
                                                                  float& OutAngleDifference)
{
    check(InAttacker);
    check(InVictim);

    const FVector VictimForward = InVictim->GetActorForwardVector();
    const FVector VictimToAttackerNormalized = (InAttacker->GetActorLocation() - InVictim->GetActorLocation()).GetSafeNormal();

    // Dot product gives the unsigned angle between VictimForward and VictimToAttacker.
    // DegAcos maps the [-1, 1] dot result to [0°, 180°].
    const float DotResult = FVector::DotProduct(VictimForward,
                                                VictimToAttackerNormalized);
    OutAngleDifference = UKismetMathLibrary::DegAcos(DotResult);

    // Cross product Z < 0 means the attacker is to the left of the victim's forward axis.
    // Negating the angle converts the range from [0°, 180°] to [-180°, +180°],
    // giving us a signed angle we can use to distinguish left from right.
    const FVector CrossResult = FVector::CrossProduct(VictimForward,
                                                      VictimToAttackerNormalized);
    if (CrossResult.Z < 0.0f)
    {
        OutAngleDifference *= -1.0f;
    }

    // Partition the signed angle into four cardinal quadrants.
    if (OutAngleDifference >= -45.0f && OutAngleDifference <= 45.0f)
    {
        return WarriorRPGTags::Shared::Status::HitReact::Front;
    }
    else if (OutAngleDifference < -45.0f && OutAngleDifference >= -135.0f)
    {
        return WarriorRPGTags::Shared::Status::HitReact::Left;
    }
    else if (OutAngleDifference < -135.0f || OutAngleDifference > 135.0f)
    {
        return WarriorRPGTags::Shared::Status::HitReact::Back;
    }
    else // OutAngleDifference > 45.0f && OutAngleDifference <= 135.0f
    {
        // All other cases mathematically fall in the right quadrant.
        return WarriorRPGTags::Shared::Status::HitReact::Right;
    }
}

bool UWarriorFunctionLibrary::IsValidBlock(AActor* InAttacker,
                                           AActor* InDefender)
{
    check(InAttacker);
    check(InDefender);

    const float DotResult = FVector::DotProduct(InAttacker->GetActorForwardVector(),
                                                InDefender->GetActorForwardVector());

    return DotResult < -0.1f;
}

bool UWarriorFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor(AActor* InInstigator,
                                                                         AActor* InTargetActor,
                                                                         const FGameplayEffectSpecHandle& InSpecHandle)
{
    // Both ASCs are treated as recoverable configuration errors, not programming
    // invariants — this function is BlueprintCallable and reachable from designer
    // graphs, where passing an actor without an ASC is a plausible authoring mistake
    // rather than a code defect. ensureMsgf logs a clear diagnostic and lets the
    // caller's bool return value drive a graceful failure path instead of crashing
    // the editor or a packaged build.
    UWarriorAbilitySystemComponent* SourceASC = NativeGetWarriorASCFromActor(InInstigator);
    if (!ensureMsgf(SourceASC,
                    TEXT("UWarriorFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor — ") TEXT( "InInstigator [%s] does not resolve to a valid ASC."),
                    *GetNameSafe(InInstigator)))
    {
        return false;
    }

    UWarriorAbilitySystemComponent* TargetASC = NativeGetWarriorASCFromActor(InTargetActor);
    if (!ensureMsgf(TargetASC,
                    TEXT("UWarriorFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor — ") TEXT( "InTargetActor [%s] does not resolve to a valid ASC."),
                    *GetNameSafe(InTargetActor)))
    {
        return false;
    }

    FActiveGameplayEffectHandle ActiveGameplayEffectHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*InSpecHandle.Data,
                                                                                                        TargetASC);

    return ActiveGameplayEffectHandle.WasSuccessfullyApplied();
}
