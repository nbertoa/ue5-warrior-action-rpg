// WarriorWeaponBase.cpp
// Implements the weapon base: creates the mesh and collision components,
// and configures the initial disabled collision state.

#include "Items/Weapons/WarriorWeaponBase.h"

#include "Components/BoxComponent.h"
#include "WarriorFunctionLibrary.h"

AWarriorWeaponBase::AWarriorWeaponBase()
{
	// Weapons are purely event-driven (animation notifies, overlaps).
	// No per-frame logic needed, so tick is disabled for performance.
	PrimaryActorTick.bCanEverTick = false;

	// --- Static mesh setup ---
	// The mesh doubles as the root component so the weapon transforms as one unit.
	// Attaching the weapon to a character socket (hand, hip) moves everything together.
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	// --- Collision box setup ---
	// Starts disabled — attack notifies will enable/disable it during active frames.
	// Attached to the mesh so it follows the weapon's transform automatically.
	// Default extent of 20 units is a placeholder; designers will resize per weapon in the Blueprint.
	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(GetRootComponent());
	WeaponCollisionBox->SetBoxExtent(FVector(20.0f));
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this,
	                                                             &ThisClass::OnCollisionBoxBeginOverlap);
	WeaponCollisionBox->OnComponentEndOverlap.AddUniqueDynamic(this,
	                                                           &ThisClass::OnCollisionBoxEndOverlap);
}

void AWarriorWeaponBase::OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                                    AActor* OtherActor,
                                                    UPrimitiveComponent* OtherComp,
                                                    int32 OtherBodyIndex,
                                                    bool bFromSweep,
                                                    const FHitResult& SweepResult)
{
	APawn* WeaponOwningPawn = GetInstigator<APawn>();
	checkf(WeaponOwningPawn,
	       TEXT("Forgot to assign an instigator as the owning pawn for the weapon: %s"),
	       *GetName());

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		// IsTargetPawnHostile replaces the previous WeaponOwningPawn != HitPawn check.
		// The old approach only prevented self-hits but allowed friendly fire between
		// allies on the same team. IsTargetPawnHostile uses IGenericTeamAgentInterface
		// to compare team IDs — only truly hostile targets trigger the hit delegate.
		if (UWarriorFunctionLibrary::IsTargetPawnHostile(WeaponOwningPawn,
		                                                 HitPawn))
		{
			OnWeaponHitTarget.ExecuteIfBound(OtherActor);
		}
	}
}

void AWarriorWeaponBase::OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                                  AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp,
                                                  int32 OtherBodyIndex)
{
	APawn* WeaponOwningPawn = GetInstigator<APawn>();
	checkf(WeaponOwningPawn,
	       TEXT("Forgot to assign an instigator as the owning pawn for the weapon: %s"),
	       *GetName());

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (UWarriorFunctionLibrary::IsTargetPawnHostile(WeaponOwningPawn,
		                                                 HitPawn))
		{
			OnWeaponPulledFromTarget.ExecuteIfBound(OtherActor);
		}
	}
}
