#pragma once

#include "BertaWorldValidation.generated.h"

/**
 * Static utility class responsible for validating actors in the currently open level.
 * Never instantiated — all entry points are static.
 *
 * Entry point:
 *   - RunValidation() — iterates all actors in the open level, reports violations to the log,
 *     and displays a summary notification. Does not modify any actor.
 *
 * Individual checks are toggled via Project Settings → Plugins → BertaDevKit → World Validation.
 */
UCLASS()
class BERTADEVKITEDITOR_API UBertaWorldValidation : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Iterates all actors in the currently open level and runs all enabled validation checks.
	 * Violations are reported to LogBertaDevKitEditor.
	 * A summary notification is displayed on completion with CS_Success or CS_Fail state.
	 */
	static void RunValidation();

private:
	/**
	 * Flags the actor if any of its UStaticMeshComponent children has no mesh asset assigned.
	 * Returns true if a violation was found.
	 */
	static bool ValidateStaticMeshComponents(const AActor* Actor);

	/**
	 * Flags the actor if its root component location exceeds the configured world bounds threshold
	 * on any axis. Returns true if a violation was found.
	 */
	static bool ValidateWorldBounds(const AActor* Actor,
	                                float BoundsThreshold);

	/**
	 * Flags the actor if it is a light and its mobility does not match the expected value
	 * configured in BertaDevKitSettings. Returns true if a violation was found.
	 */
	static bool ValidateLightMobility(const AActor* Actor,
	                                  EComponentMobility::Type ExpectedMobility);

	/**
	 * Flags the actor if its root component has a negative or zero scale on any axis.
	 * Returns true if a violation was found.
	 */
	static bool ValidateActorScale(const AActor* Actor);
};
