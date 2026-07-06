// HeroGameplayAbility_TargetLock.cpp
// Implementation of the hero target lock gameplay ability.

#include "AbilitySystem/GameplayAbilities/HeroGameplayAbility_TargetLock.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Components/SizeBox.h"
#include "Controllers/WarriorHeroController.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/WarriorActivatableWidget.h"
#include "Utils/WarriorRPGTags.h"
#include "WarriorFunctionLibrary.h"

void UHeroGameplayAbility_TargetLock::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	// Attempt to find and lock a target before calling Super —
	// if no target is found, CancelTargetLockAbility cancels before Super runs.
	TryLockOnTarget();

	InitTargetLockMovement();

	InitTargetLockMappingContext();

	Super::ActivateAbility(Handle,
	                       ActorInfo,
	                       ActivationInfo,
	                       TriggerEventData);
}

void UHeroGameplayAbility_TargetLock::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo,
                                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                                 bool bReplicateEndAbility,
                                                 bool bWasCancelled)
{
	// Do not attempt to reset input mappings during world teardown.
	// The Enhanced Input Subsystem and PlayerController may already be invalid.
	if (ActorInfo && ActorInfo->PlayerController.IsValid())
	{
		// Restore movement and input context before CleanUp — both need the hero
		// character and controller to still be valid, which CleanUp does not require.
		ResetTargetLockMovement();

		ResetTargetLockMappingContext();
	}

	// Clean up all runtime state before Super — Super triggers BP_OnEndAbility
	// and other callbacks that may query this state.
	CleanUp();

	Super::EndAbility(Handle,
	                  ActorInfo,
	                  ActivationInfo,
	                  bReplicateEndAbility,
	                  bWasCancelled);
}

void UHeroGameplayAbility_TargetLock::OnTargetLockTick(float DeltaTime)
{
	if (!CurrentLockedActor || IsTargetActorOrHeroDead())
	{
		CancelTargetLockAbility();
		return;
	}

	SetTargetLockWidgetPosition();

	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	check(HeroCharacter);

	// Suppress rotation override during roll and block — both abilities have their own
	// directional logic and should not be interrupted by the target lock camera control.
	const bool bShouldOverrideRotation = !UWarriorFunctionLibrary::NativeDoesActorHaveTag(HeroCharacter,
		WarriorRPGTags::Player::Status::Rolling) && !UWarriorFunctionLibrary::NativeDoesActorHaveTag(HeroCharacter,
		WarriorRPGTags::Player::Status::Blocking);

	if (bShouldOverrideRotation)
	{
		OverrideRotation(DeltaTime);
	}
}

bool UHeroGameplayAbility_TargetLock::IsTargetActorOrHeroDead()
{
	// Guard against a null target — the caller checks before calling this,
	// but this makes the function safe to call in isolation as well.
	if (!CurrentLockedActor)
	{
		return false;
	}

	const FGameplayTag DeadStatusTag = WarriorRPGTags::Shared::Status::Dead;

	return UWarriorFunctionLibrary::NativeDoesActorHaveTag(CurrentLockedActor,
	                                                       DeadStatusTag) ||
			UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetHeroCharacterFromActorInfo(),
			                                                DeadStatusTag);
}

void UHeroGameplayAbility_TargetLock::SwitchTarget(const FGameplayTag& InSwitchDirectionTag)
{
	// Refresh the candidate list — enemies may have moved into or out of range
	// since the last trace ran at activation time.
	UpdateAvailableActorsToLock();

	TArray<AActor*> ActorsOnLeft;
	TArray<AActor*> ActorsOnRight;
	GetAvailableActorsAroundTarget(ActorsOnLeft,
	                               ActorsOnRight);

	const bool bSwitchLeft = InSwitchDirectionTag == WarriorRPGTags::Player::Event::SwitchTarget::Left;
	const TArray<AActor*>& Candidates = bSwitchLeft
		                                    ? ActorsOnLeft
		                                    : ActorsOnRight;

	if (AActor* NewTarget = GetNearestTargetFromAvailableActors(Candidates))
	{
		CurrentLockedActor = NewTarget;
	}
	// If no candidate exists in the requested direction, keep the current target.
}

void UHeroGameplayAbility_TargetLock::TryLockOnTarget()
{
	UpdateAvailableActorsToLock();

	if (AvailableActorsToLock.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}

	CurrentLockedActor = GetNearestTargetFromAvailableActors(AvailableActorsToLock);

	if (CurrentLockedActor)
	{
		DrawTargetLockWidget();
		SetTargetLockWidgetPosition();
	}
	else
	{
		CancelTargetLockAbility();
	}
}

void UHeroGameplayAbility_TargetLock::UpdateAvailableActorsToLock()
{
	// Cache the hero character once to avoid repeated casts inside the trace call.
	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	check(HeroCharacter);

	// Clear before tracing — SwitchTarget calls this at runtime to refresh candidates,
	// so stale entries from a previous trace must not accumulate.
	AvailableActorsToLock.Empty();

	TArray<FHitResult> BoxTraceHits;

	const FVector TraceStartLocation = HeroCharacter->GetActorLocation();
	const FVector HeroFwdVector = HeroCharacter->GetActorForwardVector();
	const FVector TraceEndLocation = TraceStartLocation + HeroFwdVector * BoxTraceDistance;
	const EDrawDebugTrace::Type DrawDebugTraceType = bDebugMode
		                                                 ? EDrawDebugTrace::Persistent
		                                                 : EDrawDebugTrace::None;

	UKismetSystemLibrary::BoxTraceMultiForObjects(HeroCharacter,
	                                              TraceStartLocation,
	                                              TraceEndLocation,
	                                              TraceBoxSize * 0.5f,
	                                              HeroFwdVector.ToOrientationRotator(),
	                                              BoxTraceChannel,
	                                              false,
	                                              TArray<AActor*>(),
	                                              DrawDebugTraceType,
	                                              BoxTraceHits,
	                                              true);

	for (const FHitResult& TraceHit : BoxTraceHits)
	{
		AActor* HitActor = TraceHit.GetActor();

		// GetActor() can return nullptr if the hit primitive has no owning actor,
		// or if the actor was destroyed between the trace frame and this frame.
		// Exclude the hero itself — the trace starts at its location and may
		// return self-hits depending on the collision setup.
		if (HitActor && HitActor != HeroCharacter)
		{
			AvailableActorsToLock.AddUnique(HitActor);
		}
	}
}

AActor* UHeroGameplayAbility_TargetLock::GetNearestTargetFromAvailableActors(const TArray<AActor*>& InAvailableActors)
{
	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	check(HeroCharacter);

	// ClosestDistance is a required output parameter of FindNearestActor —
	// we discard the value here since only the actor reference is needed.
	float ClosestDistance = 0.0f;

	return UGameplayStatics::FindNearestActor(HeroCharacter->GetActorLocation(),
	                                          InAvailableActors,
	                                          ClosestDistance);
}

void UHeroGameplayAbility_TargetLock::GetAvailableActorsAroundTarget(TArray<AActor*>& OutActorsOnLeft,
                                                                     TArray<AActor*>& OutActorsOnRight)
{
	if (!CurrentLockedActor || AvailableActorsToLock.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}

	const FVector PlayerLocation = GetHeroCharacterFromActorInfo()->GetActorLocation();
	const FVector PlayerToCurrentNormalized = (CurrentLockedActor->GetActorLocation() - PlayerLocation).GetSafeNormal();

	for (AActor* AvailableActor : AvailableActorsToLock)
	{
		// Guard against actors destroyed between the trace and this classification pass.
		if (!IsValid(AvailableActor) || AvailableActor == CurrentLockedActor)
		{
			continue;
		}

		const FVector PlayerToAvailableNormalized = (AvailableActor->GetActorLocation() - PlayerLocation).
				GetSafeNormal();

		// Cross product Z > 0 means AvailableActor is clockwise from CurrentLockedActor
		// when viewed from above — i.e., to the right of the current target direction.
		const FVector CrossResult = FVector::CrossProduct(PlayerToCurrentNormalized,
		                                                  PlayerToAvailableNormalized);

		if (CrossResult.Z > 0.0f)
		{
			OutActorsOnRight.AddUnique(AvailableActor);
		}
		else
		{
			OutActorsOnLeft.AddUnique(AvailableActor);
		}
	}
}

void UHeroGameplayAbility_TargetLock::DrawTargetLockWidget()
{
	// Guard against double-draw — ActivateAbility may call TryLockOnTarget
	// more than once in edge cases (e.g., ability re-granted while active).
	if (DrawnTargetLockWidget)
	{
		return;
	}

	checkf(TargetLockWidgetClass,
	       TEXT("UHeroGameplayAbility_TargetLock::DrawTargetLockWidget — " "TargetLockWidgetClass is null. "
		       "Assign a valid widget class in the ability's Blueprint defaults."));

	DrawnTargetLockWidget = CreateWidget<UWarriorActivatableWidget>(GetHeroControllerFromActorInfo(),
	                                                                TargetLockWidgetClass);
	check(DrawnTargetLockWidget);

	DrawnTargetLockWidget->AddToViewport();
}

void UHeroGameplayAbility_TargetLock::SetTargetLockWidgetPosition()
{
	if (!DrawnTargetLockWidget || !CurrentLockedActor)
	{
		CancelTargetLockAbility();
		return;
	}

	FVector2D ScreenPosition;
	UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetHeroControllerFromActorInfo(),
	                                                           CurrentLockedActor->GetActorLocation(),
	                                                           ScreenPosition,
	                                                           true);

	// Query the widget's SizeBox once and cache the result.
	// The size is not available until after AddToViewport triggers a layout pass,
	// so this query is deferred to the first SetTargetLockWidgetPosition call.
	// Subsequent calls reuse the cached value to avoid repeated tree traversal.
	if (TargetLockWidgetSize == FVector2D::ZeroVector)
	{
		DrawnTargetLockWidget->WidgetTree->ForEachWidget([this](UWidget* FoundWidget)
		{
			if (USizeBox* FoundSizeBox = Cast<USizeBox>(FoundWidget))
			{
				TargetLockWidgetSize.X = FoundSizeBox->GetWidthOverride();
				TargetLockWidgetSize.Y = FoundSizeBox->GetHeightOverride();
			}
		});
	}

	// Offset by half the widget size to center it over the projected screen position
	// rather than anchoring its top-left corner there.
	ScreenPosition -= TargetLockWidgetSize * 0.5f;

	DrawnTargetLockWidget->SetPositionInViewport(ScreenPosition,
	                                             false);
}

void UHeroGameplayAbility_TargetLock::InitTargetLockMovement()
{
	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	check(HeroCharacter);

	UCharacterMovementComponent* HeroMovement = HeroCharacter->GetCharacterMovement();
	check(HeroMovement);

	// Cache before overriding — ResetTargetLockMovement uses this value to restore.
	CachedDefaultMaxWalkSpeed = HeroMovement->MaxWalkSpeed;

	HeroMovement->MaxWalkSpeed = TargetLockMaxWalkSpeed;
}

void UHeroGameplayAbility_TargetLock::InitTargetLockMappingContext()
{
	const ULocalPlayer* LocalPlayer = GetHeroControllerFromActorInfo()->GetLocalPlayer();
	check(LocalPlayer);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	check(Subsystem);

	// Priority 3 places this context above the default gameplay context (priority 0),
	// ensuring the target switch bindings override any conflicting default look inputs.
	Subsystem->AddMappingContext(TargetLockMappingContext,
	                             3);
}

void UHeroGameplayAbility_TargetLock::CancelTargetLockAbility()
{
	CancelAbility(GetCurrentAbilitySpecHandle(),
	              GetCurrentActorInfo(),
	              GetCurrentActivationInfo(),
	              true);
}

void UHeroGameplayAbility_TargetLock::CleanUp()
{
	AvailableActorsToLock.Empty();

	CurrentLockedActor = nullptr;

	if (DrawnTargetLockWidget)
	{
		DrawnTargetLockWidget->RemoveFromParent();
	}

	// Null unconditionally — ensures the pointer is cleared even if RemoveFromParent
	// was not reached (e.g., widget was already detached externally).
	DrawnTargetLockWidget = nullptr;

	TargetLockWidgetSize = FVector2D::ZeroVector;

	// Reset to zero so ResetTargetLockMovement can detect that Init did not complete
	// if EndAbility fires before ActivateAbility finishes (e.g., immediate cancel).
	CachedDefaultMaxWalkSpeed = 0.0f;
}

void UHeroGameplayAbility_TargetLock::ResetTargetLockMovement()
{
	// Skip if Init did not complete — CachedDefaultMaxWalkSpeed stays 0 in that case.
	if (CachedDefaultMaxWalkSpeed > 0.0f)
	{
		AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
		check(HeroCharacter);

		UCharacterMovementComponent* HeroMovement = HeroCharacter->GetCharacterMovement();
		check(HeroMovement);

		HeroMovement->MaxWalkSpeed = CachedDefaultMaxWalkSpeed;
	}
}

void UHeroGameplayAbility_TargetLock::ResetTargetLockMappingContext()
{
	// Guard against a null controller — possible if the player disconnected or
	// the controller was invalidated before EndAbility fired.
	AWarriorHeroController* HeroController = GetHeroControllerFromActorInfo();
	if (!HeroController)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = HeroController->GetLocalPlayer();
	check(LocalPlayer);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	check(Subsystem);

	Subsystem->RemoveMappingContext(TargetLockMappingContext);
}

void UHeroGameplayAbility_TargetLock::OverrideRotation(float DeltaTime)
{
	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	check(HeroCharacter);

	AWarriorHeroController* HeroController = GetHeroControllerFromActorInfo();
	check(HeroController);

	const FRotator RawLookAtRot = UKismetMathLibrary::FindLookAtRotation(HeroCharacter->GetActorLocation(),
	                                                                     CurrentLockedActor->GetActorLocation());

	// Apply camera pitch offset to keep the target visible in the lower half of the frame
	// rather than dead center — improves spatial awareness during combat.
	const FRotator LookAtRot = RawLookAtRot - FRotator(TargetLockCameraOffsetDistance,
	                                                   0.0f,
	                                                   0.0f);

	const FRotator CurrentControlRot = HeroController->GetControlRotation();
	const FRotator TargetRot = FMath::RInterpTo(CurrentControlRot,
	                                            LookAtRot,
	                                            DeltaTime,
	                                            TargetLockRotationInterpSpeed);

	// Zero out roll on both the controller and the character to prevent
	// the interpolation from introducing unwanted banking artifacts.
	HeroController->SetControlRotation(FRotator(TargetRot.Pitch,
	                                            TargetRot.Yaw,
	                                            0.0f));
	HeroCharacter->SetActorRotation(FRotator(0.0f,
	                                         TargetRot.Yaw,
	                                         0.0f));
}
