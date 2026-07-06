// BertaWorldUtils.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "BertaWorldUtils.generated.h"

/**
 * Utility library for common world queries, traces, player access, and timer helpers.
 *
 * All functions are Blueprint-callable and thread-safe where noted.
 * Functions that require a world context accept a WorldContextObject — pass 'self'
 * from any Actor or ActorComponent in Blueprint, or any valid UObject in C++.
 *
 * @note No master switch is needed here: unlike debug drawing, these functions
 *       produce no visible side effects and are safe to call in any context.
 */
UCLASS()
class BERTADEVKIT_API UBertaWorldUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ------------------------------------------------------------------
	// Actor Queries
	// ------------------------------------------------------------------

	/**
	 * Returns all actors of the given class within a sphere of the given radius
	 * centered on Origin. Optionally filters by actor tag.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param Origin              Center of the search sphere in world space.
	 * @param Radius              Search radius in Unreal units.
	 * @param ActorClass          Class to filter by. Must not be null.
	 * @param RequiredTag         If not NAME_None, only actors with this tag are included.
	 * @param OutActors           Output array of actors found within the radius.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Queries",
		meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "ActorClass", DynamicOutputParam =
			"OutActors"))
	static void GetActorsInRadius(const UObject* WorldContextObject,
	                              FVector Origin,
	                              float Radius,
	                              TSubclassOf<AActor> ActorClass,
	                              FName RequiredTag,
	                              TArray<AActor*>& OutActors);

	/**
	 * Returns the closest actor of the given class within a sphere of the given radius.
	 * Optionally filters by actor tag. Returns null if none found.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param Origin              Center of the search sphere in world space.
	 * @param Radius              Search radius in Unreal units.
	 * @param ActorClass          Class to filter by. Must not be null.
	 * @param RequiredTag         If not NAME_None, only actors with this tag are considered.
	 * @return The closest matching actor, or null if none found within radius.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Queries",
		meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "ActorClass"))
	static AActor* GetClosestActorInRadius(const UObject* WorldContextObject,
	                                       FVector Origin,
	                                       float Radius,
	                                       TSubclassOf<AActor> ActorClass,
	                                       FName RequiredTag);

	/**
	 * Returns the first actor of the given class found in the world.
	 * Optionally filters by actor tag. Order is not guaranteed.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param ActorClass          Class to filter by. Must not be null.
	 * @param RequiredTag         If not NAME_None, only actors with this tag are considered.
	 * @return The first matching actor found, or null if none exist.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Queries",
		meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "ActorClass"))
	static AActor* GetFirstActorOfClass(const UObject* WorldContextObject,
	                                    TSubclassOf<AActor> ActorClass,
	                                    FName RequiredTag);

	/**
	 * Returns all actors of the given class in the world.
	 * Optionally filters by actor tag.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param ActorClass          Class to filter by. Must not be null.
	 * @param RequiredTag         If not NAME_None, only actors with this tag are included.
	 * @param OutActors           Output array of all matching actors.
	 * @warning Iterates all actors in the world — avoid calling every frame.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Queries",
		meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "ActorClass", DynamicOutputParam =
			"OutActors"))
	static void GetAllActorsOfClass(const UObject* WorldContextObject,
	                                TSubclassOf<AActor> ActorClass,
	                                FName RequiredTag,
	                                TArray<AActor*>& OutActors);

	// ------------------------------------------------------------------
	// Traces
	// ------------------------------------------------------------------

	/**
	 * Performs a single-hit line trace from Start to End on the given collision channel.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param Start               Trace start position in world space.
	 * @param End                 Trace end position in world space.
	 * @param Channel             Collision channel to trace against.
	 * @param ActorsToIgnore      Actors excluded from the trace.
	 * @param OutHit              Hit result if the trace connects.
	 * @return True if something was hit.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Traces",
		meta = (WorldContext = "WorldContextObject"))
	static bool LineTrace(const UObject* WorldContextObject,
	                      FVector Start,
	                      FVector End,
	                      ECollisionChannel Channel,
	                      const TArray<AActor*>& ActorsToIgnore,
	                      FHitResult& OutHit);

	/**
	 * Performs a single-hit sphere trace swept from Start to End on the given collision channel.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param Start               Sweep start position in world space.
	 * @param End                 Sweep end position in world space.
	 * @param Radius              Radius of the sweep sphere in Unreal units.
	 * @param Channel             Collision channel to trace against.
	 * @param ActorsToIgnore      Actors excluded from the trace.
	 * @param OutHit              Hit result if the trace connects.
	 * @return True if something was hit.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Traces",
		meta = (WorldContext = "WorldContextObject"))
	static bool SphereTrace(const UObject* WorldContextObject,
	                        FVector Start,
	                        FVector End,
	                        float Radius,
	                        ECollisionChannel Channel,
	                        const TArray<AActor*>& ActorsToIgnore,
	                        FHitResult& OutHit);

	// ------------------------------------------------------------------
	// Player Access
	// ------------------------------------------------------------------

	/**
	 * Returns the player pawn for the given controller index. Returns null during
	 * world transitions or before BeginPlay.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param PlayerIndex         Local player index (0 for single-player).
	 * @return The player pawn, or null if not yet available.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Player",
		meta = (WorldContext = "WorldContextObject"))
	static APawn* GetPlayerPawn(const UObject* WorldContextObject,
	                            int32 PlayerIndex = 0);

	/**
	 * Returns the player controller for the given controller index. Returns null during
	 * world transitions or before BeginPlay.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param PlayerIndex         Local player index (0 for single-player).
	 * @return The player controller, or null if not yet available.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Player",
		meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetPlayerController(const UObject* WorldContextObject,
	                                              int32 PlayerIndex = 0);

	/**
	 * Returns the world-space location of the player camera.
	 * Returns FVector::ZeroVector if the camera manager is not available.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param PlayerIndex         Local player index (0 for single-player).
	 * @return Camera location in world space, or ZeroVector if unavailable.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "BertaDevKit|World|Player",
		meta = (WorldContext = "WorldContextObject"))
	static FVector GetPlayerCameraLocation(const UObject* WorldContextObject,
	                                       int32 PlayerIndex = 0);

	/**
	 * Returns the forward direction of the player camera in world space.
	 * Returns FVector::ForwardVector if the camera manager is not available.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param PlayerIndex         Local player index (0 for single-player).
	 * @return Camera forward direction (normalized), or ForwardVector if unavailable.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "BertaDevKit|World|Player",
		meta = (WorldContext = "WorldContextObject"))
	static FVector GetPlayerCameraDirection(const UObject* WorldContextObject,
	                                        int32 PlayerIndex = 0);

	// ------------------------------------------------------------------
	// Timers
	// ------------------------------------------------------------------

	/**
	 * Executes a callback after the given delay. Returns a handle that can be
	 * passed to CancelDelayedAction to cancel before it fires.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param Callback            The delegate to execute after the delay.
	 * @param Delay               Time in seconds before the callback fires. Must be > 0.
	 * @param OutHandle           Handle that can be used to cancel the timer.
	 * @warning Callback must remain valid for the duration of the delay.
	 *          If the owning object is destroyed, cancel the timer in EndPlay.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Timers",
		meta = (WorldContext = "WorldContextObject"))
	static void SetDelayedAction(const UObject* WorldContextObject,
	                             FTimerDynamicDelegate Callback,
	                             float Delay,
	                             FTimerHandle& OutHandle);

	/**
	 * Cancels a timer previously created with SetDelayedAction.
	 * Safe to call with an invalid or already-expired handle.
	 *
	 * @param WorldContextObject  Any valid UObject in the current world.
	 * @param Handle              The handle returned by SetDelayedAction.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|World|Timers",
		meta = (WorldContext = "WorldContextObject"))
	static void CancelDelayedAction(const UObject* WorldContextObject,
	                                FTimerHandle& Handle);

private:
	/**
	 * Internal helper. Returns the UWorld* from a WorldContextObject, logging
	 * an error and returning null if the context is invalid.
	 */
	static UWorld* GetWorldChecked(const UObject* WorldContextObject);

	/**
	 * Internal helper. Returns true if the actor passes the tag filter.
	 * Always returns true when RequiredTag is NAME_None.
	 */
	static bool PassesTagFilter(const AActor* Actor,
	                            FName RequiredTag);
};
