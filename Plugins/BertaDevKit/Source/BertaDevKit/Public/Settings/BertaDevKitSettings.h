#pragma once

#include "Engine/DeveloperSettings.h"
#include "BertaDevKitSettings.generated.h"

/**
 * Project-wide configuration for the BertaDevKit plugin.
 * Accessible via Project Settings → Plugins → BertaDevKit.
 * Values are persisted in Config/DefaultBertaDevKit.ini.
 */
UCLASS(Config = BertaDevKit,
	DefaultConfig,
	meta = (DisplayName = "BertaDevKit"))
class BERTADEVKIT_API UBertaDevKitSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBertaDevKitSettings();

	/**
	 * Returns the singleton instance of these settings.
	 * Safe to call from any Runtime context.
	 */
	static const UBertaDevKitSettings* Get();

	// =========================================================================
	// Debug
	// =========================================================================

	/**
	 * Master switch for the BertaDebugUtils system (PrintLog functions).
	 * When false, all PrintLog calls are skipped regardless of their individual bEnabled parameter.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "Debug",
		meta = (DisplayName = "Enable Debug Log"))
	bool bDebugLogEnabled;

	/**
	 * Master switch for the BertaDebugDraw system (Draw functions).
	 * When false, all Draw calls are skipped regardless of their individual bEnabled parameter.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "Debug",
		meta = (DisplayName = "Enable Debug Draw"))
	bool bDebugDrawEnabled;

	/**
	 * Master switch for UBertaScreenStats. When false, all Set* calls are no-ops
	 * and nothing is rendered to screen.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "Debug",
		meta = (DisplayName = "Enable Screen Stats"))
	bool bScreenStatsEnabled;

	// =========================================================================
	// World Validation
	// =========================================================================

	/**
	 * Master switch for the BertaWorldValidation system.
	 * When false, Run World Validation does nothing regardless of individual check toggles.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "World Validation",
		meta = (DisplayName = "Enable World Validation"))
	bool bWorldValidationEnabled;

	/**
	 * When true, flags actors that have a UStaticMeshComponent with no mesh asset assigned.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "World Validation",
		meta = (DisplayName = "Validate Static Mesh Components", EditCondition = "bWorldValidationEnabled"))
	bool bValidateStaticMeshComponents;

	/**
	 * When true, flags actors placed beyond the configured world bounds threshold.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "World Validation",
		meta = (DisplayName = "Validate World Bounds", EditCondition = "bWorldValidationEnabled"))
	bool bValidateWorldBounds;

	/**
	 * When true, flags light actors whose mobility does not match the expected project setting.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "World Validation",
		meta = (DisplayName = "Validate Light Mobility", EditCondition = "bWorldValidationEnabled"))
	bool bValidateLightMobility;

	/**
	 * When true, flags actors with a negative or zero scale on any axis.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "World Validation",
		meta = (DisplayName = "Validate Actor Scale", EditCondition = "bWorldValidationEnabled"))
	bool bValidateActorScale;

	/**
	 * Distance threshold in centimeters beyond which an actor is considered outside world bounds.
	 * Default is 1,000,000 cm (10 km), which covers most project scales.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "World Validation",
		meta = (DisplayName = "World Bounds Threshold (cm)", EditCondition = "bWorldValidationEnabled && bValidateWorldBounds", ClampMin = "0.0"))
	float WorldBoundsThreshold;

	/**
	 * Expected mobility for light actors in this project.
	 * Lights that do not match this value will be flagged by the mobility check.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "World Validation",
		meta = (DisplayName = "Expected Light Mobility", EditCondition = "bWorldValidationEnabled && bValidateLightMobility"))
	TEnumAsByte<EComponentMobility::Type> ExpectedLightMobility;

	//~ Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
	//~ End UDeveloperSettings Interface
};
