#include "UI/WarriorWidgetBase.h"

#include "Interfaces/PawnUIInterface.h"

void UWarriorWidgetBase::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(GetOwningPlayerPawn());

    if (!ensureMsgf(PawnUIInterface,
                    TEXT("[UWarriorWidgetBase] NativeOnInitialized — owning player pawn [%s] " "does not implement IPawnUIInterface. Verify the pawn class is correct."),
                    *GetNameSafe(GetOwningPlayerPawn())))
    {
        return;
    }

    UHeroUIComponent* HeroUIComponent = PawnUIInterface->GetHeroUIComponent();

    if (!HeroUIComponent)
    {
        return;
    }

    BP_OnOwningHeroUIComponentInitialized(HeroUIComponent);
}

void UWarriorWidgetBase::InitEnemyCreatedWidget(AActor* OwningEnemyActor)
{
    IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(OwningEnemyActor);

    if (!ensureMsgf(PawnUIInterface,
                    TEXT("[UWarriorWidgetBase] InitEnemyCreatedWidget — OwningEnemyActor [%s] " "does not implement IPawnUIInterface."),
                    *GetNameSafe(OwningEnemyActor)))
    {
        return;
    }

    UEnemyUIComponent* EnemyUIComponent = PawnUIInterface->GetEnemyUIComponent();

    if (!ensureMsgf(EnemyUIComponent,
                    TEXT("[UWarriorWidgetBase] InitEnemyCreatedWidget — EnemyUIComponent is null " "on actor [%s]."),
                    *GetNameSafe(OwningEnemyActor)))
    {
        return;
    }

    BP_OnOwningEnemyUIComponentInitialized(EnemyUIComponent);
}
