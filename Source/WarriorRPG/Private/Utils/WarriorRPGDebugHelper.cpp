// WarriorRPGDebugHelper.cpp
// Implementation of the static debug helper.
// All debug output goes through LogWarriorRPG to keep the log filterable.
// On-screen messages and log output are stripped from shipping builds automatically.

#include "Utils/WarriorRPGDebugHelper.h"

#include "Utils/WarriorRPGLogCategories.h"

void DebugHelper::Print(const FString& Msg,
                        const FColor& Color,
                        int32 InKey)
{
	// Strip all debug output in shipping builds.
	// The function signature remains intact so call sites compile without #if guards,
	// but the body does nothing — zero runtime cost in the final packaged game.
#if !UE_BUILD_SHIPPING

	// GEngine may be null during early initialization or in commandlet/server builds.
	if (GEngine)
	{
		// Display on the viewport for 7 seconds.
		// InKey == -1 means each call adds a new line; a positive key updates in-place.
		GEngine->AddOnScreenDebugMessage(InKey,
		                                 7.0f,
		                                 Color,
		                                 Msg);
	}

	// Always log regardless of GEngine availability — the log file captures
	// debug output even in headless or server scenarios where the viewport doesn't exist.
	UE_LOG(LogWarriorRPG,
	       Warning,
	       TEXT("%s"),
	       *Msg);

#endif // !UE_BUILD_SHIPPING
}

void DebugHelper::Print(const FString& FloatTitle,
                        float FloatValueToPrint,
                        int32 InKey,
                        const FColor& Color)
{
	// Strip all debug output in shipping builds.
#if !UE_BUILD_SHIPPING

	const FString FinalMsg = FloatTitle + TEXT(": ") + FString::SanitizeFloat(FloatValueToPrint);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(InKey,
		                                 7.0f,
		                                 Color,
		                                 FinalMsg);
	}

	UE_LOG(LogWarriorRPG,
	       Warning,
	       TEXT("%s"),
	       *FinalMsg);

#endif // !UE_BUILD_SHIPPING
}

void DebugHelper::Print(const FString& IntTitle,
                        int32 IntValueToPrint,
                        int32 InKey,
                        const FColor& Color)
{
	// Strip all debug output in shipping builds.
#if !UE_BUILD_SHIPPING

	// FString::FromInt converts the integer to its decimal string representation.
	// Used instead of SanitizeFloat to avoid the trailing ".0" that floats produce
	// for whole numbers — combo counts like "3.0" would be misleading on screen.
	const FString FinalMsg = IntTitle + TEXT(": ") + FString::FromInt(IntValueToPrint);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(InKey,
		                                 7.0f,
		                                 Color,
		                                 FinalMsg);
	}

	UE_LOG(LogWarriorRPG,
	       Warning,
	       TEXT("%s"),
	       *FinalMsg);

#endif // !UE_BUILD_SHIPPING
}
