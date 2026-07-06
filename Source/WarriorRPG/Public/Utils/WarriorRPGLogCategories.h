// WarriorRPGLogCategories.h
// Centralized declaration of all custom log categories for the WarriorRPG project.
// Add new categories here as systems grow (e.g., LogWarriorCombat, LogWarriorUI).

#pragma once

#include "CoreMinimal.h"

// General-purpose log category for the WarriorRPG project.
// Usage: UE_LOG(LogWarriorRPG, Log, TEXT("Message here"));
// Default verbosity: Log — compiles all verbosity levels in all builds.
DECLARE_LOG_CATEGORY_EXTERN(LogWarriorRPG,
                            Log,
                            All);
