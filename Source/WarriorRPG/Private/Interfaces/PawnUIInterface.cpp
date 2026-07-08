#include "Interfaces/PawnUIInterface.h"

UHeroUIComponent* IPawnUIInterface::GetHeroUIComponent() const
{
    return nullptr;
}

UEnemyUIComponent* IPawnUIInterface::GetEnemyUIComponent() const
{
    return nullptr;
}
