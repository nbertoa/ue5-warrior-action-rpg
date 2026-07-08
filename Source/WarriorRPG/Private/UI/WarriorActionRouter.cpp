// WarriorActionRouter.cpp

#include "UI/WarriorActionRouter.h"

void UWarriorActionRouter::ApplyUIInputConfig(const FUIInputConfig& NewConfig,
                                              bool bForceRefresh)
{
    // Intentionally empty — Common UI internally fires this with Menu mode
    // when containers initialize, which would steal input from the game.
    // Input mode changes are handled explicitly by menu widgets via
    // SetActiveUIInputConfig when they need to capture input.
}
