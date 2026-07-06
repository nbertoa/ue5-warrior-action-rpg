// WarriorHeroController.cpp
// Creates and registers the root UI layout widget with UWarriorUISubsystem
// before Super::OnPossess so OnGiven abilities can safely push widgets
// during PossessedBy without finding a null layout.

#include "Controllers/WarriorHeroController.h"

#include "Subsystems/WarriorUISubsystem.h"
#include "UI/WarriorPrimaryLayout.h"
#include "Utils/WarriorRPGLogCategories.h"

AWarriorHeroController::AWarriorHeroController()
{
	HeroTeamID = FGenericTeamId(0);
}

FGenericTeamId AWarriorHeroController::GetGenericTeamId() const
{
	return HeroTeamID;
}

void AWarriorHeroController::OnPossess(APawn* InPawn)
{
	// Non-local controllers (server, simulated) have no local player —
	// CreateWidget would crash. Skip UI creation entirely and let Super handle possession.
	if (!IsLocalController())
	{
		Super::OnPossess(InPawn);
		return;
	}

	if (!ensureMsgf(PrimaryLayoutWidgetClass,
	                TEXT("AWarriorHeroController::OnPossess — " "PrimaryLayoutWidgetClass is not set on [%s]. "
		                "Assign it in the Blueprint defaults of BP_WarriorHeroController."),
	                *GetName()))
	{
		Super::OnPossess(InPawn);
		return;
	}

	UWarriorPrimaryLayout* PrimaryLayout = CreateWidget<UWarriorPrimaryLayout>(this,
	                                                                           PrimaryLayoutWidgetClass);

	if (!ensureMsgf(PrimaryLayout,
	                TEXT("AWarriorHeroController::OnPossess — " "CreateWidget failed for class [%s]. "
		                "Verify the widget class is valid and not abstract."),
	                *PrimaryLayoutWidgetClass->GetName()))
	{
		Super::OnPossess(InPawn);
		return;
	}

	// Add to viewport before registering — the layout must be in the widget
	// tree before stacks can be registered and widgets pushed to them.
	PrimaryLayout->AddToViewport();

	UWarriorUISubsystem* UISubsystem = UWarriorUISubsystem::Get(this);

	if (!ensureMsgf(UISubsystem,
	                TEXT("AWarriorHeroController::OnPossess — " "UWarriorUISubsystem::Get returned null. "
		                "Verify the subsystem is enabled and not running on a dedicated server."),
	                *GetName()))
	{
		Super::OnPossess(InPawn);
		return;
	}

	// Register BEFORE Super::OnPossess — Super calls PossessedBy on the character,
	// which grants abilities. Abilities with OnGiven policy activate immediately
	// during GiveAbility and may push widgets to the stack.
	// The layout must be registered before that chain executes.
	UISubsystem->RegisterPrimaryLayoutWidget(PrimaryLayout);

	Super::OnPossess(InPawn);
}
