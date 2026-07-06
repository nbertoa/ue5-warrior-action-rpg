// WarriorActionRouter.h
// Custom Common UI Action Router for WarriorRPG.
// Overrides ApplyUIInputConfig to prevent Common UI from stealing input
// from the game when widgets are activated.

#pragma once

#include "CoreMinimal.h"
#include "Input/CommonUIActionRouterBase.h"
#include "WarriorActionRouter.generated.h"

/**
 * UWarriorActionRouter
 *
 * Project-specific Common UI Action Router.
 * Overrides ApplyUIInputConfig to keep the game always receiving input
 * regardless of which widgets are active on the stack.
 *
 * The mere existence of this derived class suppresses the base
 * UCommonUIActionRouterBase subsystem — no ini or Project Settings
 * configuration needed. Common UI picks this class up automatically.
 *
 * Widgets that need UI-only input (menus, dialogs) must explicitly
 * request it by calling SetActiveUIInputConfig() with the appropriate
 * FUIInputConfig from their activation logic.
 */
UCLASS()
class WARRIORRPG_API UWarriorActionRouter : public UCommonUIActionRouterBase
{
	GENERATED_BODY()

public:
	/**
	 * Overrides the input config application to always keep game input active.
	 * Must NOT call Super — completely replaces the base implementation.
	 * This prevents any widget activation from changing the input mode
	 * to UIOnly and stealing input from the game.
	 *
	 * @param NewConfig      The input config requested by the activating widget.
	 * @param bForceRefresh  Whether to force a full input refresh.
	 */
	virtual void ApplyUIInputConfig(const FUIInputConfig& NewConfig,
	                                bool bForceRefresh) override;
};
