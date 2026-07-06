// WarriorHeroController.h
// Player controller for the hero character.
// Responsible for creating and registering the root UI layout widget
// when the hero pawn is possessed, ensuring the UI is available before
// any OnGiven abilities attempt to push widgets to the stack.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "WarriorHeroController.generated.h"

class UWarriorPrimaryLayout;

/**
 * AWarriorHeroController
 *
 * Player controller for the WarriorRPG hero.
 * Creates the root UI layout widget (UWarriorPrimaryLayout) in OnPossess
 * and registers it with UWarriorUISubsystem so all subsequent widget push
 * requests can be routed to the correct stack layer.
 *
 * Implements IGenericTeamAgentInterface with team ID 0 (hero faction) so
 * AWarriorAIController::GetTeamAttitudeTowards correctly identifies the
 * hero as a hostile target for enemy AI.
 *
 * OnPossess is used instead of BeginPlay because PossessedBy on the character
 * fires before BeginPlay on the controller — abilities with the OnGiven policy
 * activate immediately during PossessedBy and may attempt to push widgets before
 * BeginPlay would have had a chance to register the layout.
 *
 * PrimaryLayoutWidgetClass must be assigned in the Blueprint defaults —
 * the controller will warn and skip UI creation if it is not set.
 */
UCLASS()
class WARRIORRPG_API AWarriorHeroController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	/**
	 * Initializes the hero team ID to 0 (hero faction).
	 * Enemy AI controllers use team ID 1 — the attitude system uses this
	 * difference to mark the hero as a hostile target for enemy perception.
	 */
	AWarriorHeroController();

	//~ Begin IGenericTeamAgentInterface Interface
	/** Returns the hero's team ID (0) for AI attitude queries. */
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~ End IGenericTeamAgentInterface Interface

protected:
	//~ Begin APlayerController Interface
	/**
	 * Called after the controller possesses a pawn and PossessedBy has completed.
	 * Creates and registers the root UI layout widget so it is available
	 * before any OnGiven abilities activate and attempt to push widgets.
	 *
	 * @param InPawn    The pawn that was just possessed.
	 */
	virtual void OnPossess(APawn* InPawn) override;
	//~ End APlayerController Interface

	/**
	 * The hero's team ID used by IGenericTeamAgentInterface.
	 * Set to 0 in the constructor — enemy controllers use 1, making
	 * attitude comparisons straightforward: lower ID = hero = hostile to enemies.
	 */
	FGenericTeamId HeroTeamID;

private:
	/**
	 * The Blueprint subclass of UWarriorPrimaryLayout to instantiate when possessing a pawn.
	 * Assign this in the Blueprint defaults of BP_WarriorHeroController.
	 * If not set, the UI system will not initialize and a warning will be logged.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "UI",
		meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UWarriorPrimaryLayout> PrimaryLayoutWidgetClass;
};
