// WarriorAIController.h
// AI Controller for all enemy characters in WarriorRPG.
// Manages AI perception (sight), team affiliation for attitude queries,
// and optional Detour Crowd Avoidance for smooth multi-agent navigation.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WarriorAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * AWarriorAIController
 *
 * Base AI controller for all enemy characters in WarriorRPG.
 * Configures sight perception to detect player-team actors and writes
 * the perceived target to the Blackboard for the behavior tree to consume.
 *
 * Team ID is set to 1 (enemy faction). The hero controller uses team ID 0,
 * so GetTeamAttitudeTowards returns Hostile for any actor with a lower team ID.
 *
 * Optionally enables Detour Crowd Avoidance via UCrowdFollowingComponent,
 * configurable per-Blueprint through the DetourCrowdAvoidance properties.
 */
UCLASS()
class WARRIORRPG_API AWarriorAIController : public AAIController
{
	GENERATED_BODY()

public:
	/**
	 * Sets up the Detour Crowd Avoidance component and configures sight perception.
	 *
	 * @param ObjectInitializer     Used to override the default path following component
	 *                              with UCrowdFollowingComponent for crowd avoidance.
	 */
	AWarriorAIController(const FObjectInitializer& ObjectInitializer);

	//~ Begin IGenericTeamAgentInterface Interface
	/**
	 * Returns the attitude of this controller toward the given actor.
	 * Hostile if the other actor's team ID is lower than ours (hero team = 0, enemy team = 1).
	 * Friendly otherwise.
	 *
	 * @param Other     The actor to evaluate attitude toward.
	 * @return          ETeamAttitude::Hostile or ETeamAttitude::Friendly.
	 */
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	//~ End IGenericTeamAgentInterface Interface

protected:
	//~ Begin AActor Interface
	/**
	 * Configures the Detour Crowd Avoidance component with the designer-specified
	 * quality and collision query range after the world is fully initialized.
	 */
	virtual void BeginPlay() override;
	//~ End AActor Interface

	/**
	 * Perception component that listens for sight stimuli.
	 * Fires OnEnemyPerceptionUpdated when a target enters or leaves the sight cone.
	 */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly)
	TObjectPtr<UAIPerceptionComponent> EnemyPerceptionComponent;

	/**
	 * Sight sense configuration attached to EnemyPerceptionComponent.
	 * Defines detection radius, peripheral angle, and affiliation filters.
	 * Configured in the constructor — override defaults in Blueprint if needed.
	 */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly)
	TObjectPtr<UAISenseConfig_Sight> AISenseConfig_Sight;

	/**
	 * Called when EnemyPerceptionComponent detects or loses a target.
	 * Writes the perceived actor to the Blackboard under the "TargetActor" key
	 * when successfully sensed, allowing the behavior tree to react.
	 *
	 * @param Actor       The actor that triggered the perception update.
	 * @param Stimulus    Contains sense type and whether the actor was sensed or lost.
	 */
	UFUNCTION()
	virtual void OnEnemyPerceptionUpdated(AActor* Actor,
	                                      FAIStimulus Stimulus);

private:
	/**
	 * Whether to enable Detour Crowd Avoidance for this AI agent.
	 * When enabled, the agent smoothly avoids other crowd agents during navigation.
	 * Disable for enemies that should not participate in crowd simulation.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Detour Crowd Avoidance Config")
	bool bEnableDetourCrowdAvoidance = true;

	/**
	 * Quality level of the Detour Crowd Avoidance simulation (1 = Low, 4 = High).
	 * Higher quality produces smoother avoidance at a higher CPU cost.
	 * Only relevant when bEnableDetourCrowdAvoidance is true.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Detour Crowd Avoidance Config",
		meta = (EditCondition = "bEnableDetourCrowdAvoidance", UIMin = "1", UIMax = "4"))
	int32 DetourCrowdAvoidanceQuality = 4;

	/**
	 * Radius in Unreal units within which this agent queries for other crowd agents.
	 * Larger values produce earlier avoidance reactions at a higher CPU cost.
	 * Only relevant when bEnableDetourCrowdAvoidance is true.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Detour Crowd Avoidance Config",
		meta = (EditCondition = "bEnableDetourCrowdAvoidance"))
	float CollisionQueryRange = 600.0f;
};
