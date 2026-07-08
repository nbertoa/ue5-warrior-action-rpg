// WarriorUISubsystem.h
// Game Instance Subsystem that manages the WarriorRPG UI system.
// Owns a reference to the root layout widget and provides the single entry
// point for pushing activatable widgets onto named stack layers.
//
// Access at runtime via UWarriorUISubsystem::Get(WorldContextObject).
// The subsystem is created automatically by the engine when the game instance
// initializes — no manual creation needed.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WarriorUISubsystem.generated.h"

class UWarriorActivatableWidget;
class UWarriorPrimaryLayout;

/**
 * UWarriorUISubsystem
 *
 * Game Instance Subsystem that serves as the central hub for the WarriorRPG UI system.
 * Holds a reference to the root UWarriorPrimaryLayout widget and routes all widget
 * push requests to the correct named stack layer via Gameplay Tags.
 *
 * Not created on dedicated servers (ShouldCreateSubsystem returns false there).
 * Not created if a subclass exists — subclasses take precedence automatically.
 */
UCLASS()
class WARRIORRPG_API UWarriorUISubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /**
     * Returns the UWarriorUISubsystem for the game instance associated with
     * the given world context object. Safe to call from any UObject with world access.
     * Returns nullptr if GEngine is not available or the subsystem does not exist.
     *
     * @param WorldContextObject    Any UObject with a valid world context.
     * @return                      The active UWarriorUISubsystem, or nullptr.
     */
    static UWarriorUISubsystem* Get(const UObject* WorldContextObject);

    //~ Begin UGameInstanceSubsystem Interface
    /**
     * Prevents creation on dedicated servers (no UI needed) and when a subclass
     * of this subsystem exists — the most-derived subclass takes precedence.
     *
     * @param Outer     The game instance that would own this subsystem.
     * @return          True if the subsystem should be created for this game instance.
     */
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    //~ End UGameInstanceSubsystem Interface

    /**
     * Stores a reference to the root layout widget created by the game's HUD or GameMode.
     * Must be called once at game startup before any PushWidgetToStack calls.
     * Crashes in development if InWidget is null.
     *
     * @param InWidget  The root layout widget instance to register.
     */
    UFUNCTION(BlueprintCallable,
        Category = "Warrior|UI")
    void RegisterPrimaryLayoutWidget(UWarriorPrimaryLayout* InWidget);

    /**
     * Instantiates InWidgetClass and pushes it onto the stack layer identified by
     * InWidgetStackTag in the registered primary layout widget.
     * The player controller is set on the widget before it is activated,
     * ensuring it is available during NativeOnActivated and BP_OnActivated.
     * Crashes in development if the primary layout has not been registered
     * or if the requested stack tag is not found in the layout.
     *
     * @param InPlayerController    The player controller to assign as the widget's owner.
     * @param InWidgetStackTag      The tag identifying the stack layer to push to.
     *                              Must belong to the WarriorRPGTags.UI.WidgetStack hierarchy.
     * @param InWidgetClass         The activatable widget class to instantiate and push.
     * @return                      The newly created and pushed widget instance.
     */
    UFUNCTION(BlueprintCallable,
        Category = "Warrior|UI")
    UWarriorActivatableWidget* PushWidgetToStack(APlayerController* InPlayerController,
                                                 UPARAM(meta = (Categories = "WarriorRPGTags.UI.WidgetStack")) FGameplayTag InWidgetStackTag,
                                                 TSubclassOf<UWarriorActivatableWidget> InWidgetClass);

private:
    /**
     * Cached reference to the root layout widget registered via RegisterPrimaryLayoutWidget.
     * Transient — not serialized. Rebuilt every session when the layout widget is created.
     */
    UPROPERTY(Transient)
    TObjectPtr<UWarriorPrimaryLayout> PrimaryLayoutWidget;
};
