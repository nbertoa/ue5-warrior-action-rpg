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

    FGameplayEventData Data;
    Data.Instigator = this;
    Data.Target = HitPawn;

    HandleProjectilePawnImpact(InstigatorPawn,
                               HitPawn,
                               Data);

    Destroy();
}

void AWarriorProjectileBase::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                                      AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp,
                                                      int32 OtherBodyIndex,
                                                      bool bFromSweep,
                                                      const FHitResult& SweepResult)
{
    if (ProjectileDamagePolicy != EProjectileDamagePolicy::OnBeginOverlap)
    {
        return;
    }

    APawn* InstigatorPawn = GetInstigator();
    if (!ensureMsgf(InstigatorPawn,
                    TEXT("AWarriorProjectileBase::OnProjectileBeginOverlap — Projectile [%s] has no Instigator set. ") TEXT("Verify the spawner calls SetInstigator before activating the projectile."),
                    *GetActorNameOrLabel()))
    {
        Destroy();
        return;
    }

    APawn* HitPawn = Cast<APawn>(OtherActor);
    if (!HitPawn || !UWarriorFunctionLibrary::IsTargetPawnHostile(InstigatorPawn,
                                                                   HitPawn))
    {
        return;
    }

    // A projectile that remains active after an overlap can receive repeated
    // overlap notifications for the same actor. Resolve each hostile pawn once.
    if (OverlappedActors.Contains(HitPawn))
    {
        return;
    }

    OverlappedActors.AddUnique(HitPawn);

    // Piercing projectiles do not generate a blocking hit, so spawn their
    // impact feedback from the overlap callback before continuing onward.
    BP_OnSpawnProjectileHitFX(SweepResult.ImpactPoint);

    FGameplayEventData Data;
    // Gameplay events consistently identify the projectile as the impact source;
    // InstigatorPawn remains the GAS effect source used for damage attribution.
    Data.Instigator = this;
    Data.Target = HitPawn;

    HandleProjectilePawnImpact(InstigatorPawn,
                               HitPawn,
                               Data);
}

void AWarriorProjectileBase::HandleProjectilePawnImpact(APawn* InInstigatorPawn,
                                                        APawn* InHitPawn,
                                                        const FGameplayEventData& InPayload)
{
    const bool bIsPlayerBlocking = UWarriorFunctionLibrary::NativeDoesActorHaveTag(InHitPawn,
                                                                                    WarriorRPGTags::Player::Status::Blocking);
    const bool bIsValidBlock = bIsPlayerBlocking && UWarriorFunctionLibrary::IsValidBlock(this,
                                                                                           InHitPawn);

    if (bIsValidBlock)
    {
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(InHitPawn,
                                                                 WarriorRPGTags::Player::Event::SuccessfulBlock,
                                                                 InPayload);
        return;
    }

    HandleApplyProjectileDamage(InInstigatorPawn,
                                InHitPawn,
                                InPayload);
}

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
