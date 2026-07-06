#include "Settings/BertaDevKitSettings.h"

UBertaDevKitSettings::UBertaDevKitSettings()
{
	// Debug systems — all enabled by default.
	// Designers can disable them globally via Project Settings → Plugins → BertaDevKit.
	bDebugLogEnabled = true;
	bDebugDrawEnabled = true;
	bScreenStatsEnabled = true;

	// World Validation — all checks enabled by default.
	// The world bounds threshold defaults to 10 km (1,000,000 cm), which covers most project scales.
	// Expected light mobility defaults to Static, the most common setting for non-dynamic projects.
	bWorldValidationEnabled = true;
	bValidateStaticMeshComponents = true;
	bValidateWorldBounds = true;
	bValidateLightMobility = true;
	bValidateActorScale = true;
	WorldBoundsThreshold = 1000000.0f;
	ExpectedLightMobility = EComponentMobility::Static;
}

const UBertaDevKitSettings* UBertaDevKitSettings::Get()
{
	return GetDefault<UBertaDevKitSettings>();
}

FName UBertaDevKitSettings::GetCategoryName() const
{
	// Groups this settings object under the "Plugins" category in Project Settings,
	// which is the conventional location for plugin configuration.
	return FName(TEXT("Plugins"));
}
