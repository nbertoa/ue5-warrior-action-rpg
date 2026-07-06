// WarriorCharacterAnimInstance.cpp
// Implements locomotion data extraction from the owning character.
// Safely reads Game Thread data in NativeUpdateAnimation and processes
// thread-safe logic in NativeThreadSafeUpdateAnimation.

#include "AnimInstances/WarriorCharacterAnimInstance.h"

#include "Characters/WarriorBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Utils/WarriorRPGLogCategories.h"

void UWarriorCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache the owning character and its movement component.
	// TryGetPawnOwner() returns the Pawn this AnimInstance is running on.
	// We cast to AWarriorBaseCharacter because we need access to project-specific APIs later.
	OwningCharacter = Cast<AWarriorBaseCharacter>(TryGetPawnOwner());

	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
	else
	{
		// This can legitimately happen in the editor when previewing animations
		// without a character, so we log at Warning rather than Error.
		UE_LOG(LogWarriorRPG,
		       Warning,
		       TEXT("UWarriorCharacterAnimInstance::NativeInitializeAnimation — "
			       "OwningCharacter is null. Animation data will not update. "
			       "This is expected in animation preview mode."));
	}
}

void UWarriorCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Early return if references are invalid — prevents crashes during hot-reload,
	// actor destruction mid-frame, or animation preview without a character.
	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}

	// GAME THREAD SAFE: We read from the character and movement component here.
	// Reading GetVelocity() or GetCurrentAcceleration() in NativeThreadSafeUpdateAnimation
	// (Worker Thread) can cause data tearing if the Game Thread updates physics at the same time.

	// Size2D() gives the magnitude of the velocity vector on the XY plane only,
	// ignoring vertical velocity (falling, jumping). This is what we want for
	// ground locomotion blending — a character falling at 500 units/sec shouldn't
	// trigger the run animation.
	GroundSpeed = OwningCharacter->GetVelocity().Size2D();

	// SizeSquared2D() > 0 is cheaper than Size2D() > 0 because it avoids a square root.
	// Any non-zero acceleration means the player is actively providing movement input.
	// This distinction matters for the AnimGraph: a character with GroundSpeed > 0 but
	// bHasAcceleration == false is sliding to a stop (deceleration), and should play
	// a deceleration/stop animation rather than the walk/run loop.
	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.0f;

	// CalculateDirection returns the angle in degrees between the actor's forward vector
	// and its current velocity — positive = moving right, negative = moving left.
	// Used in the AnimGraph to drive strafe blendspaces for 8-directional locomotion.
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(OwningCharacter->GetVelocity(),
	                                                                  OwningCharacter->GetActorRotation());
}

void UWarriorCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// WORKER THREAD SAFE: Perform heavy animation math, logic, or state transitions here.
	// For the base character, GroundSpeed and bHasAcceleration are already prepared
	// by NativeUpdateAnimation, so child classes (like WarriorHeroAnimInstance)
	// can safely use them here without touching game thread components.
}
