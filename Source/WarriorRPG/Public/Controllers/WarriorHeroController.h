// WarriorHeroController.h
// Player controller responsible for the hero's local UI root and team identity.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "WarriorHeroController.generated.h"

class UWarriorPrimaryLayout;

/**
 * Player controller for the hero character.
 *
 * Owns a single root UI layout for the local player and registers it with
 * UWarriorUISubsystem. The layout is created before local startup abilities
 * are granted and is reused across pawn re-possessions.
 */
UCLASS()
class WARRIORRPG_API AWarriorHeroController : public APlayerController, public IGenericTeamAgentInterface
{
    GENERATED_BODY()

public:
    /** Initializes the hero team ID to 0. */
    AWarriorHeroController();

    //~ Begin IGenericTeamAgentInterface Interface
    /** Returns the hero's team ID. */
    virtual FGenericTeamId GetGenericTeamId() const override;
    //~ End IGenericTeamAgentInterface Interface

    /** Creates and registers the local player's root layout when needed. */
    void EnsurePrimaryLayoutWidget();

protected:
    //~ Begin APlayerController Interface
    /** Initializes the local UI when this controller enters the playing state. */
    virtual void BeginPlayingState() override;

    /** Removes the registered root layout before this controller is destroyed. */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End APlayerController Interface

    /** Team identifier used by AI attitude queries. */
    FGenericTeamId HeroTeamID;

private:
    /** Blueprint layout class assigned in the controller defaults. */
    UPROPERTY(EditDefaultsOnly,
        Category = "UI",
        meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UWarriorPrimaryLayout> PrimaryLayoutWidgetClass;

    /** Root layout retained for the controller's play session. */
    UPROPERTY(Transient)
    TObjectPtr<UWarriorPrimaryLayout> PrimaryLayoutWidget;
};
