// WarriorHeroController.cpp

#include "Controllers/WarriorHeroController.h"

#include "Subsystems/WarriorUISubsystem.h"
#include "UI/WarriorPrimaryLayout.h"

AWarriorHeroController::AWarriorHeroController()
{
    HeroTeamID = FGenericTeamId(0);
}

FGenericTeamId AWarriorHeroController::GetGenericTeamId() const
{
    return HeroTeamID;
}

void AWarriorHeroController::EnsurePrimaryLayoutWidget()
{
    // Server-side controllers for remote players do not own a viewport.
    if (!IsLocalController())
    {
        return;
    }

    UWarriorUISubsystem* UISubsystem = UWarriorUISubsystem::Get(this);
    if (!ensureMsgf(UISubsystem,
                    TEXT("AWarriorHeroController::EnsurePrimaryLayoutWidget: UWarriorUISubsystem is unavailable on [%s]."),
                    *GetName()))
    {
        return;
    }

    // Keep one root layout for the controller's entire play session.
    if (IsValid(PrimaryLayoutWidget))
    {
        UISubsystem->RegisterPrimaryLayoutWidget(PrimaryLayoutWidget);
        return;
    }

    if (!ensureMsgf(PrimaryLayoutWidgetClass,
                    TEXT("AWarriorHeroController::EnsurePrimaryLayoutWidget: PrimaryLayoutWidgetClass is not set on [%s]."),
                    *GetName()))
    {
        return;
    }

    PrimaryLayoutWidget = CreateWidget<UWarriorPrimaryLayout>(this,
                                                              PrimaryLayoutWidgetClass);
    if (!ensureMsgf(PrimaryLayoutWidget,
                    TEXT("AWarriorHeroController::EnsurePrimaryLayoutWidget: CreateWidget failed for class [%s]."),
                    *PrimaryLayoutWidgetClass->GetName()))
    {
        return;
    }

    PrimaryLayoutWidget->AddToViewport();
    UISubsystem->RegisterPrimaryLayoutWidget(PrimaryLayoutWidget);
}

void AWarriorHeroController::BeginPlayingState()
{
    Super::BeginPlayingState();

    EnsurePrimaryLayoutWidget();
}

void AWarriorHeroController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(PrimaryLayoutWidget))
    {
        PrimaryLayoutWidget->RemoveFromParent();

        if (UWarriorUISubsystem* UISubsystem = UWarriorUISubsystem::Get(this))
        {
            UISubsystem->UnregisterPrimaryLayoutWidget(PrimaryLayoutWidget);
        }

        PrimaryLayoutWidget = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}
