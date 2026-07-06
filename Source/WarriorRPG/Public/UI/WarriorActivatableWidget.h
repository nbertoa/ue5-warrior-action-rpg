// WarriorActivatableWidget.h
// Base class for all activatable widgets in WarriorRPG.
// Extends UCommonActivatableWidget with a cached, type-safe getter for the
// hero player controller, automatic hero UI component resolution on activation,
// and a designer-configurable input mode system based on the Lyra pattern.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "WarriorActivatableWidget.generated.h"

class AWarriorHeroController;
class UEnemyUIComponent;
class UHeroUIComponent;

/**
 * Defines how a widget interacts with game input when activated.
 * Set this per-widget in Blueprint defaults to control whether the game
 * still receives input while this widget is on screen.
 *
 * Default    — no input config change. Game receives all input (use for HUD, overlays).
 * Game       — game receives input, mouse captured. Use for in-game widgets that
 *              coexist with gameplay (e.g., inventory that doesn't pause).
 * GameAndMenu — game and UI both receive input. Use for hybrid states.
 * Menu       — UI only, no capture. Use for pause menus, main menu, options.
 */
UENUM(BlueprintType)
enum class EWarriorWidgetInputMode : uint8
{
	Default,
	Game,
	GameAndMenu,
	Menu
};

/**
 * UWarriorActivatableWidget
 *
 * Base class for all activatable widgets in WarriorRPG.
 * Inherits Common UI's activation/deactivation lifecycle and input routing
 * from UCommonActivatableWidget, and adds:
 *
 * - A designer-configurable input mode (EWarriorWidgetInputMode) that controls
 *   whether the game receives input while this widget is active.
 *   Defaults to Default — no input mode change, game receives everything.
 * - A cached getter for the hero player controller to avoid repeated casting.
 * - Automatic resolution of the owning pawn's UHeroUIComponent on activation,
 *   exposed to Blueprint via BP_OnOwningHeroUIComponentInitialized.
 * - Manual initialization for enemy-owned widgets via InitEnemyCreatedWidget.
 *
 * All project widgets should inherit from this class rather than
 * UCommonActivatableWidget directly.
 */
UCLASS(Abstract,
	BlueprintType,
	meta = (DisableNativeTick))
class WARRIORRPG_API UWarriorActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/**
	 * Initializes this widget as an enemy-owned widget by resolving the
	 * UEnemyUIComponent from the given enemy actor and forwarding it to Blueprint.
	 * Call this manually after creating a widget that is owned by an enemy pawn
	 * rather than the player — enemy widgets are not pushed via UWarriorUISubsystem
	 * and do not go through the normal NativeOnActivated hero resolution path.
	 *
	 * Crashes in development if OwningEnemyActor is null, does not implement
	 * IPawnUIInterface, or does not have a valid UEnemyUIComponent.
	 *
	 * @param OwningEnemyActor  The enemy actor that owns this widget. Must not be null
	 *                          and must implement IPawnUIInterface.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|UI")
	void InitEnemyCreatedWidget(AActor* OwningEnemyActor);

protected:
	//~ Begin UCommonActivatableWidget Interface
	/**
	 * Returns the input config this widget requests when activated.
	 * Driven by InputConfig and GameMouseCaptureMode — set these in Blueprint defaults
	 * to control how this widget interacts with game input.
	 * Default returns an empty TOptional — no input mode change, game receives everything.
	 */
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	/**
	 * Called when this widget is activated and fully visible.
	 * Resolves the owning pawn's UHeroUIComponent and forwards it to Blueprint
	 * via BP_OnOwningHeroUIComponentInitialized so delegates can be bound.
	 */
	virtual void NativeOnActivated() override;
	//~ End UCommonActivatableWidget Interface

	/**
	 * Called during NativeOnActivated when the owning pawn implements IPawnUIInterface
	 * and returns a valid UHeroUIComponent.
	 * Implement this in Blueprint to bind health, rage, and other UI delegates.
	 *
	 * @param OwningHeroUIComponent     Guaranteed non-null when this event fires.
	 */
	UFUNCTION(BlueprintImplementableEvent,
		Category = "Warrior|UI",
		meta = (DisplayName = "On Owning Hero UI Component Initialized"))
	void BP_OnOwningHeroUIComponentInitialized(UHeroUIComponent* OwningHeroUIComponent);

	/**
	 * Called by InitEnemyCreatedWidget when a valid UEnemyUIComponent is found.
	 * Implement this in Blueprint to bind enemy-specific UI delegates.
	 *
	 * @param OwningEnemyUIComponent    Guaranteed non-null when this event fires.
	 */
	UFUNCTION(BlueprintImplementableEvent,
		Category = "Warrior|UI",
		meta = (DisplayName = "On Owning Enemy UI Component Initialized"))
	void BP_OnOwningEnemyUIComponentInitialized(UEnemyUIComponent* OwningEnemyUIComponent);

	/**
	 * Returns the hero player controller that owns this widget, cached on first access.
	 *
	 * @return The owning AWarriorHeroController, or nullptr if destroyed.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|UI")
	AWarriorHeroController* GetOwningWarriorHeroController();

	/**
	 * Controls how this widget interacts with game input when activated.
	 * Default = no change, game receives everything.
	 * Set to Menu for pause menus, options, main menu.
	 * Set to Game or GameAndMenu for in-game overlays that need some UI interaction.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Input")
	EWarriorWidgetInputMode InputConfig = EWarriorWidgetInputMode::Default;

	/**
	 * Mouse capture behavior when InputConfig is Game or GameAndMenu.
	 * Has no effect when InputConfig is Default or Menu.
	 * CapturePermanently is the correct setting for most gameplay scenarios.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "Input")
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;

private:
	TWeakObjectPtr<AWarriorHeroController> CachedOwningPC;
};
