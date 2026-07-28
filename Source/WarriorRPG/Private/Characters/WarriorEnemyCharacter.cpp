// WarriorEnemyCharacter.cpp

#include "Characters/WarriorEnemyCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "Components/UI/EnemyUIComponent.h"
#include "Components/WidgetComponent.h"
#include "DataAssets/StartupData/DataAsset_EnemyStartupData.h"
#include "Engine/AssetManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/WarriorWidgetBase.h"
#include "Utils/WarriorRPGLogCategories.h"
#include "WarriorFunctionLibrary.h"

AWarriorEnemyCharacter::AWarriorEnemyCharacter()
{
    // Automatically possessed by an AIController when placed in the world
    // or spawned dynamically at runtime. No manual possession needed.
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Disable controller rotation influence on the character's facing direction.
    // Enemies orient themselves to their movement direction instead,
    // which produces natural-looking AI locomotion without snapping.
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    bUseControllerRotationYaw = false;

    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (ensureMsgf(IsValid(MovementComp),
                   TEXT("[AWarriorEnemyCharacter] CharacterMovementComponent is null — check base class setup.")))
    {
        // Orient to movement: the character smoothly rotates to face its velocity
        // direction. Combined with RotationRate this avoids the "ice skating" look
        // of instant direction changes.
        MovementComp->bUseControllerDesiredRotation = false;
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->RotationRate = FRotator(0.0f,
                                              180.0f,
                                              0.0f);

        // Conservative speed values for melee enemies.
        // Fast enough to be threatening, slow enough to be readable.
        MovementComp->MaxWalkSpeed = 300.0f;
        MovementComp->BrakingDecelerationWalking = 1000.0f;
    }

    EnemyCombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("EnemyCombatComponent"));
    EnemyUIComponent = CreateDefaultSubobject<UEnemyUIComponent>(TEXT("EnemyUIComponent"));

    // Attached to the mesh so the health bar follows the character in world space.
    EnemyHealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHealthWidgetComponent"));
    EnemyHealthWidgetComponent->SetupAttachment(GetMesh());

    // Both hand boxes share a single overlap callback — registered via AddUniqueDynamic
    // so binding the same function twice does not produce duplicate calls.
    LeftHandCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftHandCollisionBox"));
    LeftHandCollisionBox->SetupAttachment(GetMesh());
    LeftHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftHandCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this,
                                                                   &ThisClass::OnBodyCollisionBoxBeginOverlap);

    RightHandCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHandCollisionBox"));
    RightHandCollisionBox->SetupAttachment(GetMesh());
    RightHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightHandCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this,
                                                                    &ThisClass::OnBodyCollisionBoxBeginOverlap);
}

UPawnCombatComponent* AWarriorEnemyCharacter::GetPawnCombatComponent() const
{
    return EnemyCombatComponent;
}

UPawnUIComponent* AWarriorEnemyCharacter::GetPawnUIComponent() const
{
    return EnemyUIComponent;
}

UEnemyUIComponent* AWarriorEnemyCharacter::GetEnemyUIComponent() const
{
    return EnemyUIComponent;
}

void AWarriorEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    check(EnemyHealthWidgetComponent);

    // Cast instead of CastChecked — the widget class is assigned by the designer
    // in Blueprint and may legitimately not be a UWarriorActivatableWidget
    // (e.g., during early development or when using a placeholder widget class).
    UUserWidget* Widget = EnemyHealthWidgetComponent->GetUserWidgetObject();
    if (UWarriorWidgetBase* HealthWidget = Cast<UWarriorWidgetBase>(Widget))
    {
        HealthWidget->InitEnemyCreatedWidget(this);
    }
}

void AWarriorEnemyCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    InitEnemyStartupData();
}

#if WITH_EDITOR
void AWarriorEnemyCharacter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Re-attach the left hand box to the updated bone immediately so the
    // designer can see the result in the viewport without restarting the editor.
    if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass,
                                                                                LeftHandCollisionBoxAttachBoneName))
    {
        LeftHandCollisionBox->AttachToComponent(GetMesh(),
                                                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                                                LeftHandCollisionBoxAttachBoneName);
    }

    // Same pattern for the right hand box.
    if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass,
                                                                                RightHandCollisionBoxAttachBoneName))
    {
        RightHandCollisionBox->AttachToComponent(GetMesh(),
                                                 FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                                                 RightHandCollisionBoxAttachBoneName);
    }
}
#endif

void AWarriorEnemyCharacter::OnBodyCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                                            AActor* OtherActor,
                                                            UPrimitiveComponent* OtherComp,
                                                            int32 OtherBodyIndex,
                                                            bool bFromSweep,
                                                            const FHitResult& SweepResult)
{
    // Only process hits against other pawns — ignore static geometry,
    // projectiles, and any non-pawn actors that enter the collision box.
    if (APawn* HitPawn = Cast<APawn>(OtherActor))
    {
        // IsTargetPawnHostile uses IGenericTeamAgentInterface to check faction
        // affiliation, replacing the naive self-comparison used in earlier iterations.
        if (UWarriorFunctionLibrary::IsTargetPawnHostile(this,
                                                         HitPawn))
        {
            EnemyCombatComponent->OnHitTargetActor(HitPawn);
        }
    }
}

void AWarriorEnemyCharacter::InitEnemyStartupData()
{
    if (bStartupDataApplied || bStartupDataLoadInFlight)
    {
        return;
    }

    if (!ensureMsgf(!CharacterStartupData.IsNull(),
                    TEXT("[AWarriorEnemyCharacter] CharacterStartupData is null on %s — " "assign it in the Blueprint defaults."),
                    *GetName()))
    {
        return;
    }

    bStartupDataLoadInFlight = true;

    // Async load keeps the game thread unblocked — the callback fires as soon
    // as the asset is ready, which is typically within the same frame on a warm cache.
    UAssetManager::GetStreamableManager().RequestAsyncLoad(CharacterStartupData.ToSoftObjectPath(),
                                                           FStreamableDelegate::CreateUObject(this,
                                                                                              &ThisClass::OnStartupDataLoaded));
}

void AWarriorEnemyCharacter::OnStartupDataLoaded()
{
    bStartupDataLoadInFlight = false;

    if (bStartupDataApplied)
    {
        return;
    }

    if (UDataAsset_StartupDataBase* LoadedData = CharacterStartupData.Get())
    {
        LoadedData->GiveToAbilitySystemComponent(WarriorAbilitySystemComponent);
        bStartupDataApplied = true;

        UE_LOG(LogWarriorRPG,
               Log,
               TEXT("[AWarriorEnemyCharacter] Startup data loaded and applied on %s."),
               *GetName());
    }
    else
    {
        // CharacterStartupData.Get() returned null after a successful async load request —
        // this typically means the asset was unloaded or garbage collected between the
        // request and the callback (e.g., during a level transition or PIE stop).
        UE_LOG(LogWarriorRPG,
               Warning,
               TEXT("[AWarriorEnemyCharacter] Startup data asset failed to load on %s. " "The enemy will have no abilities or attributes."),
               *GetName());
    }
}
