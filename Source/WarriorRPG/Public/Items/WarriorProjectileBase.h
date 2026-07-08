#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "WarriorProjectileBase.generated.h"

class UBoxComponent;
class UNiagaraComponent;
class UProjectileMovementComponent;
struct FGameplayEventData;

UENUM(BlueprintType)
enum class EProjectileDamagePolicy : uint8
{
    OnHit,
    OnBeginOverlap
};

UCLASS()
class WARRIORRPG_API AWarriorProjectileBase : public AActor
{
    GENERATED_BODY()

public:
    AWarriorProjectileBase();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleDefaultsOnly,
        BlueprintReadOnly,
        Category = "Projectile")
    TObjectPtr<UBoxComponent> ProjectileCollisionBox;

    UPROPERTY(VisibleDefaultsOnly,
        BlueprintReadOnly,
        Category = "Projectile")
    TObjectPtr<UNiagaraComponent> ProjectileNiagaraComponent;

    UPROPERTY(VisibleDefaultsOnly,
        BlueprintReadOnly,
        Category = "Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;

    UPROPERTY(EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Settings")
    EProjectileDamagePolicy ProjectileDamagePolicy = EProjectileDamagePolicy::OnHit;

    UPROPERTY(BlueprintReadOnly,
        Category = "Settings",
        meta = (ExposeOnSpawn = "true"))
    FGameplayEffectSpecHandle ProjectileDamageEffectSpecHandle;

    UFUNCTION()
    virtual void OnProjectileHit(UPrimitiveComponent* HitComponent,
                                 AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp,
                                 FVector NormalImpulse,
                                 const FHitResult& Hit);

    UFUNCTION()
    virtual void OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                          AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp,
                                          int32 OtherBodyIndex,
                                          bool bFromSweep,
                                          const FHitResult& SweepResult);

    UFUNCTION(BlueprintImplementableEvent,
        meta = (DisplayName = "On Spawn Projectile Hit FX"))
    void BP_OnSpawnProjectileHitFX(const FVector& HitLocation);

private:
    /**
     * Applies the projectile's damage effect spec to the hit pawn and, on success,
     * sends the shared HitReact gameplay event so the target's hit react ability
     * (granted at startup, activated via WaitGameplayEvent) can trigger.
     *
     * @param InInstigatorPawn  The pawn that fired this projectile. Already validated
     *                          non-null by the caller (OnProjectileHit) — never re-checked here.
     * @param InHitPawn         The pawn that was struck. Must not be null.
     * @param InPayload         Event data forwarded to the HitReact gameplay event on success.
     */
    void HandleApplyProjectileDamage(APawn* InInstigatorPawn,
                                     APawn* InHitPawn,
                                     const FGameplayEventData& InPayload);
};
