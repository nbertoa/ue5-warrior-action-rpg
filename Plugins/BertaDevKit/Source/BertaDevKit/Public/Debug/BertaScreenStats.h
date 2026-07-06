#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaScreenStats.generated.h"

/**
 * Represents a single named entry in the on-screen stats panel.
 */
USTRUCT(BlueprintType)
struct BERTADEVKIT_API FBertaStatEntry
{
	GENERATED_BODY()

	/** Display name shown on the left side of the panel. */
	UPROPERTY(BlueprintReadOnly,
		Category = "BertaDevKit|ScreenStats")
	FString DisplayName;

	/** Current value shown on the right side of the panel. */
	UPROPERTY(BlueprintReadOnly,
		Category = "BertaDevKit|ScreenStats")
	FString Value;

	/** Color used to render this entry. Defaults to white. */
	UPROPERTY(BlueprintReadOnly,
		Category = "BertaDevKit|ScreenStats")
	FLinearColor Color = FLinearColor::White;
};

/**
 * On-screen stats panel for real-time display of named values during development.
 *
 * Maintains a persistent key-value panel rendered every frame via GEngine.
 * Each named stat occupies exactly one line — calling Set() with the same name
 * updates the existing entry rather than adding a duplicate.
 *
 * All functions respect the master switch BertaDevKitSettings::bScreenStatsEnabled.
 * The panel is fully stripped in shipping builds via DevelopmentOnly.
 *
 * Usage:
 *   UBertaScreenStats::SetFloat("Player Speed", Velocity.Size());
 *   UBertaScreenStats::SetInt("Enemies In Range", EnemyCount);
 *   UBertaScreenStats::SetBool("Is Grounded", bIsGrounded);
 */
UCLASS()
class BERTADEVKIT_API UBertaScreenStats : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ------------------------------------------------------------------
	// Set — Float
	// ------------------------------------------------------------------

	/**
	 * Registers or updates a named float stat on the screen panel.
	 * Displayed as a decimal with the given number of decimal places.
	 *
	 * @param Name          Unique identifier for this stat entry.
	 * @param Value         Float value to display.
	 * @param DecimalPlaces Number of decimal places shown (default: 2).
	 * @param Color         Display color for this entry.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|ScreenStats",
		meta = (DevelopmentOnly, AdvancedDisplay = "DecimalPlaces,Color"))
	static void SetFloat(FName Name,
	                     float Value,
	                     int32 DecimalPlaces = 2,
	                     FLinearColor Color = FLinearColor::White);

	// ------------------------------------------------------------------
	// Set — Int
	// ------------------------------------------------------------------

	/**
	 * Registers or updates a named integer stat on the screen panel.
	 *
	 * @param Name   Unique identifier for this stat entry.
	 * @param Value  Integer value to display.
	 * @param Color  Display color for this entry.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|ScreenStats",
		meta = (DevelopmentOnly, AdvancedDisplay = "Color"))
	static void SetInt(FName Name,
	                   int32 Value,
	                   FLinearColor Color = FLinearColor::White);

	// ------------------------------------------------------------------
	// Set — Bool
	// ------------------------------------------------------------------

	/**
	 * Registers or updates a named boolean stat on the screen panel.
	 * Displayed as "true" / "false". Color defaults to green/red based on value
	 * unless overridden.
	 *
	 * @param Name          Unique identifier for this stat entry.
	 * @param Value         Boolean value to display.
	 * @param bAutoColor    If true, green = true, red = false. Ignores Color param.
	 * @param Color         Display color override when bAutoColor is false.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|ScreenStats",
		meta = (DevelopmentOnly, AdvancedDisplay = "bAutoColor,Color"))
	static void SetBool(FName Name,
	                    bool Value,
	                    bool bAutoColor = true,
	                    FLinearColor Color = FLinearColor::White);

	// ------------------------------------------------------------------
	// Set — String
	// ------------------------------------------------------------------

	/**
	 * Registers or updates a named string stat on the screen panel.
	 *
	 * @param Name   Unique identifier for this stat entry.
	 * @param Value  String value to display.
	 * @param Color  Display color for this entry.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|ScreenStats",
		meta = (DevelopmentOnly, AdvancedDisplay = "Color"))
	static void SetString(FName Name,
	                      const FString& Value,
	                      FLinearColor Color = FLinearColor::White);

	// ------------------------------------------------------------------
	// Set — Vector
	// ------------------------------------------------------------------

	/**
	 * Registers or updates a named FVector stat on the screen panel.
	 * Displayed as "X=N Y=N Z=N" with the given decimal places.
	 *
	 * @param Name          Unique identifier for this stat entry.
	 * @param Value         Vector value to display.
	 * @param DecimalPlaces Number of decimal places per component (default: 1).
	 * @param Color         Display color for this entry.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|ScreenStats",
		meta = (DevelopmentOnly, AdvancedDisplay = "DecimalPlaces,Color"))
	static void SetVector(FName Name,
	                      FVector Value,
	                      int32 DecimalPlaces = 1,
	                      FLinearColor Color = FLinearColor::White);

	// ------------------------------------------------------------------
	// Panel Control
	// ------------------------------------------------------------------

	/**
	 * Removes a single named entry from the panel.
	 * Safe to call if the entry does not exist.
	 *
	 * @param Name  The stat name to remove.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|ScreenStats",
		meta = (DevelopmentOnly))
	static void Remove(FName Name);

	/**
	 * Removes all entries from the panel.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|ScreenStats",
		meta = (DevelopmentOnly))
	static void Clear();

private:
	/**
	 * Internal setter used by all public Set* functions.
	 * Creates a new entry or updates an existing one with the given name.
	 */
	static void SetEntry(FName Name,
	                     const FString& FormattedValue,
	                     FLinearColor Color);

	/**
	 * Registers the RenderStats callback to FCoreDelegates::OnBeginFrame.
	 * Called lazily on the first Set* call — avoids registering if the system
	 * is never used in a given session.
	 *
	 * Uses a static local flag internally to prevent double-registration.
	 * This is safer than a class-level static member because initialization
	 * is guaranteed to occur on first call, avoiding static initialization order issues.
	 */
	static void EnsureRegistered();

	/**
	 * Renders all current entries to the screen via GEngine->AddOnScreenDebugMessage.
	 * Bound to FCoreDelegates::OnBeginFrame. Respects bScreenStatsEnabled master switch.
	 *
	 * @note This function is safe to call from a headless (no-render) context because
	 *       it guards on GEngine before drawing. In server builds GEngine may be null.
	 */
	static void RenderStats();

	/**
	 * Persistent map of stat entries keyed by name.
	 * Defined as a static local inside the .cpp to avoid static initialization order issues.
	 */
	static TMap<FName, FBertaStatEntry>& GetEntries();
};
