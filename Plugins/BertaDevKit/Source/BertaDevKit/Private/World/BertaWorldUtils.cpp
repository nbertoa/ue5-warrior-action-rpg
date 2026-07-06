// BertaWorldUtils.cpp
#include "World/BertaWorldUtils.h"
#include "Log/BertaDevKitLog.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"

// --------------------------------------------------------------------
// Internal Helpers
// --------------------------------------------------------------------

UWorld* UBertaWorldUtils::GetWorldChecked(const UObject* WorldContextObject)
{
	// WorldContextObject can be null if the caller passes an invalid self reference —
	// guard before dereferencing to avoid a crash with no stack trace.
	if (!IsValid(WorldContextObject))
	{
		UE_LOG(LogBertaDevKit,
		       Error,
		       TEXT("[BertaWorldUtils] WorldContextObject is null or pending kill."));
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		UE_LOG(LogBertaDevKit,
		       Error,
		       TEXT("[BertaWorldUtils] Could not retrieve UWorld from context object '%s'."),
		       *WorldContextObject->GetName());
	}

	return World;
}

bool UBertaWorldUtils::PassesTagFilter(const AActor* Actor,
                                       const FName RequiredTag)
{
	// NAME_None is the sentinel value meaning "no filter" — always pass.
	if (RequiredTag == NAME_None)
	{
		return true;
	}

	return Actor->ActorHasTag(RequiredTag);
}

// --------------------------------------------------------------------
// Actor Queries
// --------------------------------------------------------------------

void UBertaWorldUtils::GetActorsInRadius(const UObject* WorldContextObject,
                                         const FVector Origin,
                                         const float Radius,
                                         const TSubclassOf<AActor> ActorClass,
                                         const FName RequiredTag,
                                         TArray<AActor*>& OutActors)
{
	OutActors.Reset();

	UWorld* World = GetWorldChecked(WorldContextObject);
	if (!World)
	{
		return;
	}

	if (!ActorClass)
	{
		UE_LOG(LogBertaDevKit,
		       Warning,
		       TEXT("[BertaWorldUtils::GetActorsInRadius] ActorClass is null — returning empty array."));
		return;
	}

	// Precompute squared radius to avoid a sqrt per actor during distance check.
	const float RadiusSquared = Radius * Radius;

	for (TActorIterator<AActor> It(World,
	                               ActorClass); It; ++It)
	{
		AActor* Actor = *It;

		if (!IsValid(Actor))
		{
			continue;
		}

		// Distance check in squared space — cheaper than FVector::Dist.
		const float DistanceSquared = FVector::DistSquared(Origin,
		                                                   Actor->GetActorLocation());
		if (DistanceSquared > RadiusSquared)
		{
			continue;
		}

		if (!PassesTagFilter(Actor,
		                     RequiredTag))
		{
			continue;
		}

		OutActors.Add(Actor);
	}
}

AActor* UBertaWorldUtils::GetClosestActorInRadius(const UObject* WorldContextObject,
                                                  const FVector Origin,
                                                  const float Radius,
                                                  const TSubclassOf<AActor> ActorClass,
                                                  const FName RequiredTag)
{
	UWorld* World = GetWorldChecked(WorldContextObject);
	if (!World)
	{
		return nullptr;
	}

	if (!ActorClass)
	{
		UE_LOG(LogBertaDevKit,
		       Warning,
		       TEXT("[BertaWorldUtils::GetClosestActorInRadius] ActorClass is null — returning null."));
		return nullptr;
	}

	AActor* ClosestActor = nullptr;
	float ClosestDistanceSquared = Radius * Radius;

	for (TActorIterator<AActor> It(World,
	                               ActorClass); It; ++It)
	{
		AActor* Actor = *It;

		if (!IsValid(Actor))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(Origin,
		                                                   Actor->GetActorLocation());

		// Reject actors outside the current closest distance first.
		// Strict less-than: on a tie, the first actor found wins.
		if (DistanceSquared >= ClosestDistanceSquared)
		{
			continue;
		}

		// Tag filter is checked after distance to avoid the string comparison
		// for actors that are already farther than the current best.
		if (!PassesTagFilter(Actor,
		                     RequiredTag))
		{
			continue;
		}

		// Both distance and tag checks passed — this is the new closest candidate.
		ClosestActor = Actor;
		ClosestDistanceSquared = DistanceSquared;
	}

	return ClosestActor;
}

AActor* UBertaWorldUtils::GetFirstActorOfClass(const UObject* WorldContextObject,
                                               const TSubclassOf<AActor> ActorClass,
                                               const FName RequiredTag)
{
	UWorld* World = GetWorldChecked(WorldContextObject);
	if (!World)
	{
		return nullptr;
	}

	if (!ActorClass)
	{
		UE_LOG(LogBertaDevKit,
		       Warning,
		       TEXT("[BertaWorldUtils::GetFirstActorOfClass] ActorClass is null — returning null."));
		return nullptr;
	}

	for (TActorIterator<AActor> It(World,
	                               ActorClass); It; ++It)
	{
		AActor* Actor = *It;

		if (!IsValid(Actor))
		{
			continue;
		}

		if (!PassesTagFilter(Actor,
		                     RequiredTag))
		{
			continue;
		}

		// Return immediately on the first match — no need to iterate further.
		return Actor;
	}

	return nullptr;
}

void UBertaWorldUtils::GetAllActorsOfClass(const UObject* WorldContextObject,
                                           const TSubclassOf<AActor> ActorClass,
                                           const FName RequiredTag,
                                           TArray<AActor*>& OutActors)
{
	OutActors.Reset();

	UWorld* World = GetWorldChecked(WorldContextObject);
	if (!World)
	{
		return;
	}

	if (!ActorClass)
	{
		UE_LOG(LogBertaDevKit,
		       Warning,
		       TEXT("[BertaWorldUtils::GetAllActorsOfClass] ActorClass is null — returning empty array."));
		return;
	}

	for (TActorIterator<AActor> It(World,
	                               ActorClass); It; ++It)
	{
		AActor* Actor = *It;

		if (!IsValid(Actor))
		{
			continue;
		}

		if (!PassesTagFilter(Actor,
		                     RequiredTag))
		{
			continue;
		}

		OutActors.Add(Actor);
	}
}

// --------------------------------------------------------------------
// Traces
// --------------------------------------------------------------------

bool UBertaWorldUtils::LineTrace(const UObject* WorldContextObject,
                                 const FVector Start,
                                 const FVector End,
                                 const ECollisionChannel Channel,
                                 const TArray<AActor*>& ActorsToIgnore,
                                 FHitResult& OutHit)
{
	UWorld* World = GetWorldChecked(WorldContextObject);
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams QueryParams;

	// Ignore complex collision on ignored actors — avoids false positives
	// on actors whose simple collision bounds contain the trace start point.
	QueryParams.AddIgnoredActors(ActorsToIgnore);
	QueryParams.bTraceComplex = false;

	return World->LineTraceSingleByChannel(OutHit,
	                                       Start,
	                                       End,
	                                       Channel,
	                                       QueryParams);
}

bool UBertaWorldUtils::SphereTrace(const UObject* WorldContextObject,
                                   const FVector Start,
                                   const FVector End,
                                   const float Radius,
                                   const ECollisionChannel Channel,
                                   const TArray<AActor*>& ActorsToIgnore,
                                   FHitResult& OutHit)
{
	UWorld* World = GetWorldChecked(WorldContextObject);
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(ActorsToIgnore);
	QueryParams.bTraceComplex = false;

	// SweepSingleByChannel takes a shape as FCollisionShape — build it here
	// rather than making the caller know about FCollisionShape internals.
	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);

	return World->SweepSingleByChannel(OutHit,
	                                   Start,
	                                   End,
	                                   FQuat::Identity,
	                                   Channel,
	                                   SphereShape,
	                                   QueryParams);
}

// --------------------------------------------------------------------
// Player Access
// --------------------------------------------------------------------

APawn* UBertaWorldUtils::GetPlayerPawn(const UObject* WorldContextObject,
                                       const int32 PlayerIndex)
{
	// Delegate to UGameplayStatics — it already handles null world gracefully
	// and is the canonical way to access local player state.
	return UGameplayStatics::GetPlayerPawn(WorldContextObject,
	                                       PlayerIndex);
}

APlayerController* UBertaWorldUtils::GetPlayerController(const UObject* WorldContextObject,
                                                         const int32 PlayerIndex)
{
	return UGameplayStatics::GetPlayerController(WorldContextObject,
	                                             PlayerIndex);
}

FVector UBertaWorldUtils::GetPlayerCameraLocation(const UObject* WorldContextObject,
                                                  const int32 PlayerIndex)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject,
	                                                                    PlayerIndex);

	if (!IsValid(PC) || !IsValid(PC->PlayerCameraManager))
	{
		UE_LOG(LogBertaDevKit,
		       Warning,
		       TEXT("[BertaWorldUtils::GetPlayerCameraLocation] "
			       "PlayerController or CameraManager not available for index %d. Returning ZeroVector."),
		       PlayerIndex);
		return FVector::ZeroVector;
	}

	return PC->PlayerCameraManager->GetCameraLocation();
}

FVector UBertaWorldUtils::GetPlayerCameraDirection(const UObject* WorldContextObject,
                                                   const int32 PlayerIndex)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject,
	                                                                    PlayerIndex);

	if (!IsValid(PC) || !IsValid(PC->PlayerCameraManager))
	{
		UE_LOG(LogBertaDevKit,
		       Warning,
		       TEXT("[BertaWorldUtils::GetPlayerCameraDirection] "
			       "PlayerController or CameraManager not available for index %d. Returning ForwardVector."),
		       PlayerIndex);
		return FVector::ForwardVector;
	}

	// GetActorForwardVector on CameraManager returns the camera's current forward —
	// equivalent to the view direction but expressed as a unit vector.
	return PC->PlayerCameraManager->GetActorForwardVector();
}

// --------------------------------------------------------------------
// Timers
// --------------------------------------------------------------------

void UBertaWorldUtils::SetDelayedAction(const UObject* WorldContextObject,
                                        const FTimerDynamicDelegate Callback,
                                        const float Delay,
                                        FTimerHandle& OutHandle)
{
	UWorld* World = GetWorldChecked(WorldContextObject);
	if (!World)
	{
		return;
	}

	if (!Callback.IsBound())
	{
		UE_LOG(LogBertaDevKit,
		       Warning,
		       TEXT("[BertaWorldUtils::SetDelayedAction] Callback delegate is not bound — timer not set."));
		return;
	}

	if (Delay <= 0.0f)
	{
		UE_LOG(LogBertaDevKit,
		       Warning,
		       TEXT("[BertaWorldUtils::SetDelayedAction] Delay is %.2f — must be > 0. Timer not set."),
		       Delay);
		return;
	}

	// bLoop = false — this is a one-shot timer.
	World->GetTimerManager().SetTimer(OutHandle,
	                                  Callback,
	                                  Delay,
	                                  false);
}

void UBertaWorldUtils::CancelDelayedAction(const UObject* WorldContextObject,
                                           FTimerHandle& Handle)
{
	UWorld* World = GetWorldChecked(WorldContextObject);
	if (!World)
	{
		return;
	}

	// ClearTimer is safe to call on an invalid or already-expired handle —
	// no guard needed here, the TimerManager handles it internally.
	World->GetTimerManager().ClearTimer(Handle);
}
