// WarriorCharacterAnimInstance.h
// AnimInstance that provides locomotion data to the Animation Blueprint.
// Reads movement state from the owning character every frame and exposes
// it as Blueprint-readable properties for blending between animation states.

#pragma once

#include "CoreMinimal.h"
#include "AnimInstances/WarriorBaseAnimInstance.h"
#include "WarriorCharacterAnimInstance.generated.h"

class UCharacterMovementComponent;
class AWarriorBaseCharacter;

/**
 * Character AnimInstance that drives locomotion blending.
 * Caches references to the owning character and its movement component,
 * then updates ground speed and acceleration state every animation tick.
 *
 * Thread-safety improvement: Game thread data (Velocity, Acceleration) is read
 * in NativeUpdateAnimation, while internal animation state calculation is
 * deferred to NativeThreadSafeUpdateAnimation on the worker thread.
 */
UCLASS()
class WARRIORRPG_API UWarriorCharacterAnimInstance : public UWarriorBaseAnimInstance
{
	GENERATED_BODY()

public:
	//~ Begin UAnimInstance Interface
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	//~ End UAnimInstance Interface

protected:
	/** Cached reference to the owning WarriorBaseCharacter. Set once during initialization. */
	UPROPERTY()
	TObjectPtr<AWarriorBaseCharacter> OwningCharacter;

	/** Cached reference to the owning character's movement component for efficient per-frame access. */
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> OwningMovementComponent;

	/**
	 * Current horizontal movement speed of the character (ignores vertical velocity).
	 * Used in the AnimGraph to blend between idle, walk, and run states.
	 */
	UPROPERTY(VisibleDefaultsOnly,
		BlueprintReadOnly,
		Category = "AnimData|LocomotionData")
	float GroundSpeed;

	/**
	 * Whether the character is currently accelerating (has movement input).
	 * Used to distinguish between actively moving and decelerating/sliding to a stop.
	 */
	UPROPERTY(VisibleDefaultsOnly,
		BlueprintReadOnly,
		Category = "AnimData|LocomotionData")
	bool bHasAcceleration;

	/**
	 * The locomotion direction angle of the character in degrees relative to its facing direction.
	 * Computed via UKismetAnimationLibrary::CalculateDirection from velocity and actor rotation.
	 * Used in the AnimGraph to drive strafe blendspaces — positive values indicate rightward
	 * movement, negative values indicate leftward movement.
	 */
	UPROPERTY(VisibleDefaultsOnly,
		BlueprintReadOnly,
		Category = "AnimData|LocomotionData")
	float LocomotionDirection;
};
