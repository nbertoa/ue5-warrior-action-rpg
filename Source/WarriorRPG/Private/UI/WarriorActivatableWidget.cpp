// WarriorActivatableWidget.cpp

#include "UI/WarriorActivatableWidget.h"

#include "Controllers/WarriorHeroController.h"
#include "Interfaces/PawnUIInterface.h"

void UWarriorActivatableWidget::InitEnemyCreatedWidget(AActor* OwningEnemyActor)
{
	check(OwningEnemyActor);

	IPawnUIInterface* PawnUIInterface = CastChecked<IPawnUIInterface>(OwningEnemyActor);

	UEnemyUIComponent* EnemyUIComponent = PawnUIInterface->GetEnemyUIComponent();
	checkf(EnemyUIComponent,
	       TEXT("UWarriorActivatableWidget::InitEnemyCreatedWidget — "
		       "Failed to extract a UEnemyUIComponent from [%s]. "
		       "Verify the enemy character creates the component in its constructor."),
	       *OwningEnemyActor->GetActorNameOrLabel());

	BP_OnOwningEnemyUIComponentInitialized(EnemyUIComponent);
}

TOptional<FUIInputConfig> UWarriorActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EWarriorWidgetInputMode::Game:
		// Game receives input, mouse captured — for in-game overlays that
		// coexist with gameplay without pausing or blocking game input.
		return FUIInputConfig(ECommonInputMode::Game,
		                      GameMouseCaptureMode);

	case EWarriorWidgetInputMode::GameAndMenu:
		// Both game and UI receive input — for hybrid states like an
		// inventory that can be used while the game is still running.
		return FUIInputConfig(ECommonInputMode::All,
		                      GameMouseCaptureMode);

	case EWarriorWidgetInputMode::Menu:
		// UI only, no mouse capture — for pause menus, main menu, options.
		// Game receives no input while this widget is active.
		return FUIInputConfig(ECommonInputMode::Menu,
		                      EMouseCaptureMode::NoCapture);

	case EWarriorWidgetInputMode::Default: default:
		// No input config change — game receives everything.
		// Correct default for HUD widgets and overlays that only display data.
		return TOptional<FUIInputConfig>();
	}
}

void UWarriorActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(GetOwningPlayerPawn()))
	{
		if (UHeroUIComponent* HeroUIComponent = PawnUIInterface->GetHeroUIComponent())
		{
			BP_OnOwningHeroUIComponentInitialized(HeroUIComponent);
		}
	}
}

AWarriorHeroController* UWarriorActivatableWidget::GetOwningWarriorHeroController()
{
	if (!CachedOwningPC.IsValid())
	{
		CachedOwningPC = GetOwningPlayer<AWarriorHeroController>();
	}

	return CachedOwningPC.IsValid()
		       ? CachedOwningPC.Get()
		       : nullptr;
}
