// WarriorPrimaryLayout.cpp
// Implements widget stack registration and lookup for UWarriorPrimaryLayout.

#include "UI/WarriorPrimaryLayout.h"

#include "Utils/WarriorRPGLogCategories.h"

UCommonActivatableWidgetContainerBase* UWarriorPrimaryLayout::FindWidgetStackByTag(const FGameplayTag& InTag) const
{
    // Crash immediately if the requested stack was never registered —
    // this means the Blueprint forgot to call RegisterWidgetStack for this layer,
    // which is always a setup error that must be caught before shipping.
    checkf(WidgetStackByTag.Contains(InTag),
           TEXT("UWarriorPrimaryLayout::FindWidgetStackByTag — " "No widget stack registered under tag [%s]. " "Ensure RegisterWidgetStack is called for this layer during widget initialization."),
           *InTag.ToString());

    return WidgetStackByTag.FindRef(InTag);
}

void UWarriorPrimaryLayout::RegisterWidgetStack(FGameplayTag InStackTag,
                                                UCommonActivatableWidgetContainerBase* InStack)
{
    // Null stack is always a Blueprint wiring error — crash fast in development.
    check(InStack);

    // Skip registration during design time to prevent spurious entries
    // created while the widget Blueprint is open in the editor.
    if (IsDesignTime())
    {
        return;
    }

    if (WidgetStackByTag.Contains(InStackTag))
    {
        // Duplicate registration — log a warning and skip rather than overwriting
        // the existing stack, which could silently orphan widgets already pushed to it.
        UE_LOG(LogWarriorRPG,
               Warning,
               TEXT("UWarriorPrimaryLayout::RegisterWidgetStack — " "Stack [%s] is already registered. Ignoring duplicate registration."),
               *InStackTag.GetTagName().ToString());
        return;
    }

    WidgetStackByTag.Add(InStackTag,
                         InStack);

    UE_LOG(LogWarriorRPG,
           Log,
           TEXT("UWarriorPrimaryLayout::RegisterWidgetStack — " "Stack [%s] registered successfully."),
           *InStackTag.GetTagName().ToString());
}
