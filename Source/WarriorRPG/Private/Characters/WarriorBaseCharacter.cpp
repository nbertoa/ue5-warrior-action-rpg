// WarriorBaseCharacter.cpp
// Base constructor configures shared defaults for all character types.

#include "Characters/WarriorBaseCharacter.h"

#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "MotionWarpingComponent.h"
#include "Utils/WarriorRPGLogCategories.h"

AWarriorBaseCharacter::AWarriorBaseCharacter()
{
    // Disable tick by default for performance — most game logic should be event-driven.
    // Subclasses can re-enable tick if they have a genuine need (e.g., continuous aiming trace).
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // Disable decal rendering on the character mesh.
    // Prevents visual artifacts from ground decals (blood splats, AoE indicators)
    // projecting onto the character model.
    GetMesh()->bReceivesDecals = false;

    // --- Gameplay Ability System setup ---
    // Both the ASC and AttributeSet are created as default subobjects so they exist
    // from the moment the character is constructed. The ASC manages abilities, effects,
    // and tags; the AttributeSet holds the actual numeric attributes (health, stamina, etc.).
    WarriorAbilitySystemComponent = CreateDefaultSubobject<UWarriorAbilitySystemComponent>(TEXT("WarriorAbilitySystemComponent"));

    WarriorAttributeSet = CreateDefaultSubobject<UWarriorAttributeSet>(TEXT("WarriorAttributeSet"));

    // Created on the base character so both hero and enemy abilities can use motion warping
    // uniformly without duplicating the component on each subclass. Any ability that needs
    // precise positional alignment (attack lunges, finishing moves) can warp to a target
    // without needing hand-authored animation variants per character type.
    MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

UAbilitySystemComponent* AWarriorBaseCharacter::GetAbilitySystemComponent() const
{
    // IAbilitySystemInterface requires this function to return the base UAbilitySystemComponent*.
    // We delegate to our typed getter for consistency.
    return GetWarriorAbilitySystemComponent();
}

UPawnCombatComponent* AWarriorBaseCharacter::GetPawnCombatComponent() const
{
    return nullptr;
}

UPawnUIComponent* AWarriorBaseCharacter::GetPawnUIComponent() const
{
    return nullptr;
}

UHeroUIComponent* AWarriorBaseCharacter::GetHeroUIComponent() const
{
    return nullptr;
}

UEnemyUIComponent* AWarriorBaseCharacter::GetEnemyUIComponent() const
{
    return nullptr;
}

void AWarriorBaseCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // Initialize the ASC actor info before anything else — abilities granted later
    // depend on this being set up correctly.
    if (!ensureMsgf(WarriorAbilitySystemComponent,
                    TEXT("%s: WarriorAbilitySystemComponent is null in PossessedBy. " "This should never happen — the component is created in the constructor."),
                    *GetName()))
    {
        return;
    }

    // Owner = this character, Avatar = this character (single-player setup).
    // In multiplayer with ASC on PlayerState, Owner would be the PlayerState instead.
    WarriorAbilitySystemComponent->InitAbilityActorInfo(this,
                                                        this);

    // Warn if the designer forgot to assign startup data.
    // We don't return here because the character can still function without startup data
    // (it just won't have any initial abilities), and subclasses may handle the case differently.
    ensureMsgf(!CharacterStartupData.IsNull(),
               TEXT("%s: CharacterStartupData is not assigned. " "Assign a StartupData asset in the Blueprint defaults to grant initial abilities."),
               *GetName());
}
