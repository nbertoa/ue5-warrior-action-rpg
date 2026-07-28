// WarriorUISubsystem.cpp
// Implements the central UI management subsystem for WarriorRPG.

#include "Subsystems/WarriorUISubsystem.h"

#include "UI/WarriorActivatableWidget.h"
#include "UI/WarriorPrimaryLayout.h"
#include "Utils/WarriorRPGLogCategories.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

UWarriorUISubsystem* UWarriorUISubsystem::Get(const UObject* WorldContextObject)
{
    if (!GEngine)
    {
        return nullptr;
    }

    // Assert mode: if WorldContextObject has no world, this is a programming error
    // and we want to know about it immediately rather than silently returning null.
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,
                                                       EGetWorldErrorMode::Assert);

    return UGameInstance::GetSubsystem<UWarriorUISubsystem>(World->GetGameInstance());
}

bool UWarriorUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Never create on dedicated servers — no UI is needed and Common UI
    // assets may not be loaded in a server-only cook.
    if (CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
    {
        return false;
    }

    // If a subclass of this subsystem exists, let the most-derived class
    // take ownership. The engine will instantiate that subclass instead.
    TArray<UClass*> DerivedClasses;
    GetDerivedClasses(GetClass(),
                      DerivedClasses);

    return DerivedClasses.IsEmpty();
}

void UWarriorUISubsystem::RegisterPrimaryLayoutWidget(UWarriorPrimaryLayout* InWidget)
{
    // Null layout is always a caller error — the subsystem cannot function without it.
    check(InWidget);

    PrimaryLayoutWidget = InWidget;

    UE_LOG(LogWarriorRPG,
           Log,
           TEXT("UWarriorUISubsystem::RegisterPrimaryLayoutWidget — " "Primary layout widget [%s] registered successfully."),
           *InWidget->GetName());
}

void UWarriorUISubsystem::UnregisterPrimaryLayoutWidget(UWarriorPrimaryLayout* InWidget)
{
    if (PrimaryLayoutWidget == InWidget)
    {
        PrimaryLayoutWidget = nullptr;
    }
}

UWarriorActivatableWidget* UWarriorUISubsystem::PushWidgetToStack(APlayerController* InPlayerController,
                                                                  FGameplayTag InWidgetStackTag,
                                                                  TSubclassOf<UWarriorActivatableWidget> InWidgetClass)
{
    checkf(InPlayerController,
           TEXT("UWarriorUISubsystem::PushWidgetToStack — InPlayerController is null. " "Widgets pushed to a player stack require an owning controller."));
    checkf(InWidgetStackTag.IsValid(),
           TEXT("UWarriorUISubsystem::PushWidgetToStack — InWidgetStackTag is invalid. " "Pass a registered WarriorRPGTags.UI.WidgetStack tag."));
    checkf(InWidgetClass,
           TEXT("UWarriorUISubsystem::PushWidgetToStack — InWidgetClass is null. " "Pass a valid non-abstract activatable widget class."));

    // Hard invariant: RegisterPrimaryLayoutWidget must be called at game startup
    // before any widget push requests. A null layout means that call was skipped.
    checkf(PrimaryLayoutWidget,
           TEXT("UWarriorUISubsystem::PushWidgetToStack — " "PrimaryLayoutWidget is null. " "Ensure RegisterPrimaryLayoutWidget is called at game startup."));

    // Locate the correct stack layer by tag — crashes if the tag is not registered,
    // which means the Blueprint forgot to call RegisterWidgetStack for this layer.
    UCommonActivatableWidgetContainerBase* TargetStack = PrimaryLayoutWidget->FindWidgetStackByTag(InWidgetStackTag);
    checkf(TargetStack,
           TEXT("UWarriorUISubsystem::PushWidgetToStack — Registered stack [%s] is null. " "Verify the layout's stack registration."),
           *InWidgetStackTag.ToString());

    // AddWidget instantiates the widget and pushes it onto the stack.
    // The lambda runs before the widget is activated, guaranteeing the player
    // controller is set and available during NativeOnActivated and BP_OnActivated.
    // Setting it after AddWidget would be too late — the widget may already be
    // trying to access the controller during its activation chain.
    UWarriorActivatableWidget* NewWidget = TargetStack->AddWidget<UWarriorActivatableWidget>(InWidgetClass,
                                                                                             [InPlayerController](UWarriorActivatableWidget& WidgetToInit)
                                                                                             {
                                                                                                 WidgetToInit.SetOwningPlayer(InPlayerController);
                                                                                             });

    checkf(NewWidget,
           TEXT("UWarriorUISubsystem::PushWidgetToStack — " "AddWidget returned null for class [%s]. " "Verify the widget class is valid and not abstract."),
           *InWidgetClass->GetName());

    return NewWidget;
}
