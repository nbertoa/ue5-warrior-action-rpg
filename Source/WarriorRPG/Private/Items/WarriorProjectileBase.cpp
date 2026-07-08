#include "Items/WarriorProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "WarriorFunctionLibrary.h"
#include "Utils/WarriorRPGTags.h"

AWarriorProjectileBase::AWarriorProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    ProjectileCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ProjectileCollisionBox"));
    SetRootComponent(ProjectileCollisionBox);
    ProjectileCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn,
                                                          ECR_Block);
    ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic,
                                                          ECR_Block);
    ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic,
                                                          ECR_Block);
    ProjectileCollisionBox->OnComponentHit.AddUniqueDynamic(this,
                                                            &ThisClass::OnProjectileHit);
    ProjectileCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this,
                                                                     &ThisClass::OnProjectileBeginOverlap);

    ProjectileNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));
    ProjectileNiagaraComponent->SetupAttachment(GetRootComponent());

    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
    ProjectileMovementComp->InitialSpeed = 700.0f;
    ProjectileMovementComp->MaxSpeed = 900.0f;
    ProjectileMovementComp->Velocity = FVector(1.0f,
                                               0.0f,
                                               0.0f);
    ProjectileMovementComp->ProjectileGravityScale = 0.0f;

    InitialLifeSpan = 4.0f;
}

void AWarriorProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    if (ProjectileDamagePolicy == EProjectileDamagePolicy::OnBeginOverlap)
    {
        ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn,
                                                              ECR_Overlap);
    }
}

void AWarriorProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent,
                                             AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp,
                                             FVector NormalImpulse,
                                             const FHitResult& Hit)
{
    check(OtherActor);

    BP_OnSpawnProjectileHitFX(Hit.ImpactPoint);

    // The instigator is validated once, here, and threaded through to every
    // downstream call (IsTargetPawnHostile, HandleApplyProjectileDamage) instead
    // of each one calling GetInstigator() independently. Both of those callees
    // hard-check their pawn argument internally — a null instigator there would
    // crash the process instead of failing gracefully.
    APawn* InstigatorPawn = GetInstigator();
    if (!ensureMsgf(InstigatorPawn,
                    TEXT("AWarriorProjectileBase::OnProjectileHit — Projectile [%s] has no Instigator set. ") TEXT( "Verify the spawner calls SetInstigator before activating the projectile."),
                    *GetActorNameOrLabel()))
    {
        Destroy();
        return;
    }

    APawn* HitPawn = Cast<APawn>(OtherActor);

    // Named for what it actually holds: true when this hit should be ignored,
    // either because OtherActor is not a pawn at all, or because it is not hostile
    // to the instigator (friendly fire / self-hit).
    const bool bShouldIgnoreHit = !HitPawn || !UWarriorFunctionLibrary::IsTargetPawnHostile(InstigatorPawn,
                                                                                            HitPawn);
    if (bShouldIgnoreHit)
    {
        Destroy();
        return;
    }

    bool bIsValidBlock = false;

    const bool bIsPlayerBlocking = UWarriorFunctionLibrary::NativeDoesActorHaveTag(HitPawn,
                                                                                   WarriorRPGTags::Player::Status::Blocking);
    if (bIsPlayerBlocking)
    {
        bIsValidBlock = UWarriorFunctionLibrary::IsValidBlock(this,
                                                              HitPawn);
    }

    FGameplayEventData Data;
    Data.Instigator = this;
    Data.Target = HitPawn;

    if (bIsValidBlock)
    {
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitPawn,
                                                                 WarriorRPGTags::Player::Event::SuccessfulBlock,
                                                                 Data);
    }
    else
    {
        HandleApplyProjectileDamage(InstigatorPawn,
                                    HitPawn,
                                    Data);
    }

    Destroy();
}

void AWarriorProjectileBase::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                                      AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp,
                                                      int32 OtherBodyIndex,
                                                      bool bFromSweep,
                                                      const FHitResult& SweepResult) {}

void AWarriorProjectileBase::HandleApplyProjectileDamage(APawn* InInstigatorPawn,
                                                         APawn* InHitPawn,
                                                         const FGameplayEventData& InPayload)
{
    checkf(ProjectileDamageEffectSpecHandle.IsValid(),
           TEXT("Forgot to assign a valid spec handle to the projectile: %s"),
           *GetActorNameOrLabel());

    const bool bWasApplied = UWarriorFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor(InInstigatorPawn,
                                                                                                 InHitPawn,
                                                                                                 ProjectileDamageEffectSpecHandle);

    if (bWasApplied)
    {
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(InHitPawn,
                                                                 WarriorRPGTags::Shared::Event::HitReact,
                                                                 InPayload);
    }
}
