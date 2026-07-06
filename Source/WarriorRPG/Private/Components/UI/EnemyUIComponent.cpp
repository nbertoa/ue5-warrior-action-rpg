// EnemyUIComponent.cpp
// Enemy-specific UI component implementation.
// Manages the lifecycle of widgets drawn by this enemy instance.

#include "Components/UI/EnemyUIComponent.h"

#include "UI/WarriorActivatableWidget.h"

void UEnemyUIComponent::RegisterEnemyDrawnWidget(UWarriorActivatableWidget* InWidget)
{
	// Defensive: a null widget would produce a silent gap in the tracking array,
	// causing RemoveEnemyDrawnWidgetsIfAny to skip a widget that was never cleaned up.
	if (!ensureMsgf(IsValid(InWidget),
	                TEXT("[UEnemyUIComponent] RegisterEnemyDrawnWidget called with a null or "
		                "pending-kill widget on %s. Verify the widget was created successfully "
		                "before registering it."),
	                *GetOwner()->GetName()))
	{
		return;
	}

	EnemyDrawnWidgets.Add(InWidget);
}

void UEnemyUIComponent::RemoveEnemyDrawnWidgetsIfAny()
{
	if (EnemyDrawnWidgets.IsEmpty())
	{
		return;
	}

	for (UWarriorActivatableWidget* DrawnWidget : EnemyDrawnWidgets)
	{
		// IsValid guard: the widget may have been collected or explicitly destroyed
		// between registration and this cleanup call (e.g., manual RemoveFromParent
		// elsewhere, or GC during a level transition). Skip rather than crash.
		if (!IsValid(DrawnWidget))
		{
			continue;
		}

		DrawnWidget->RemoveFromParent();
	}

	EnemyDrawnWidgets.Empty();
}
