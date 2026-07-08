// WarriorPrimaryLayout.h
// Root layout widget for the WarriorRPG UI system.
// Manages multiple named widget stack layers identified by Gameplay Tags.
// Each stack is a UCommonActivatableWidgetContainerBase that holds and
// activates widgets pushed by UWarriorUISubsystem::PushWidgetToStack.
//
// One instance of this widget is created at game start and registered with
// UWarriorUISubsystem via RegisterPrimaryLayoutWidget. All subsequent
// widget push/pop operations route through this layout.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "WarriorPrimaryLayout.generated.h"

/**
 * UWarriorPrimaryLayout
 *
 * Root UI layout widget for WarriorRPG.
 * Owns a registry of named widget stack layers (background, HUD, modal, toasts)
 * identified by Gameplay Tags from the WarriorRPGTags::UI::WidgetStack namespace.
 *
 * Stack registration is performed from Blueprint during widget initialization
 * via RegisterWidgetStack. UWarriorUISubsystem queries stacks by tag via
 * FindWidgetStackByTag when pushing new widgets.
 *
 * Only one instance of this widget should exist at a time — it is created
 * and registered by the game's HUD or GameMode Blueprint at startup.
 */
UCLASS(Abstract,
    BlueprintType,
    meta = (DisableNativeTick))
class WARRIORRPG_API UWarriorPrimaryLayout : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    /**
     * Finds and returns the widget stack container registered under the given tag.
     * Called by UWarriorUISubsystem::PushWidgetToStack to locate the correct layer.
     * Crashes in development if no stack has been registered under InTag —
     * this indicates the Blueprint forgot to call RegisterWidgetStack for that layer.
     *
     * @param InTag     The Gameplay Tag identifying the desired stack layer.
     *                  Must belong to the WarriorRPGTags.UI.WidgetStack hierarchy.
     * @return          The stack container registered under InTag.
     */
    UCommonActivatableWidgetContainerBase* FindWidgetStackByTag(const FGameplayTag& InTag) const;

protected:
    /**
     * Registers a widget stack container under the given Gameplay Tag.
     * Called from Blueprint during widget initialization — one call per stack layer.
     * Logs a warning and skips registration if the tag is already registered,
     * preventing accidental overwrites of existing stacks.
     * No-op during editor design time to prevent spurious registrations in the
     * widget Blueprint editor.
     *
     * @param InStackTag    The tag to register this stack under.
     *                      Must belong to the WarriorRPGTags.UI.WidgetStack hierarchy.
     * @param InStack       The stack container to register. Must not be null.
     */
    UFUNCTION(BlueprintCallable,
        Category = "Warrior|UI")
    void RegisterWidgetStack(UPARAM(meta = (Categories = "WarriorRPGTags.UI.WidgetStack")) FGameplayTag InStackTag,
                             UCommonActivatableWidgetContainerBase* InStack);

private:
    /**
     * Registry of widget stack containers indexed by Gameplay Tag.
     * Populated at runtime by RegisterWidgetStack during widget initialization.
     * Transient — not serialized or saved to disk, rebuilt every session.
     * TObjectPtr ensures GC tracking of the container references.
     */
    UPROPERTY(Transient)
    TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> WidgetStackByTag;
};
