#pragma once

#include "AssetRegistry/AssetData.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "BertaAssetNamingUtils.generated.h"

/**
 * Result of a single asset rename attempt performed by UBertaAssetNamingUtils::RenameAssetWithPrefix().
 * Used by callers to log and report outcomes without magic booleans.
 *
 * @note This enum is C++-only by design. If Blueprint exposure is needed in the future,
 *       add BlueprintType to the UENUM specifier and expose RenameAssetWithPrefix via UFUNCTION.
 */
UENUM()
enum class EBertaRenameResult : uint8
{
	/** Asset was successfully renamed with the correct prefix. */
	Renamed,

	/** Asset already carried the correct prefix — no action taken. */
	AlreadyCorrect,

	/** No prefix entry exists in the map for this asset's class — asset was skipped. */
	UnknownClass
};

/**
 * Sole owner of the asset prefix map and all prefix-related logic.
 * Consumed by UBertaAssetNamingActions and UBertaAssetAuditor.
 *
 * @note All functions are static — this class is never instantiated.
 */
UCLASS()
class BERTADEVKITEDITOR_API UBertaAssetNamingUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Returns the canonical map of UClass to expected name prefix.
	 * Stored as a static local — built once, reused across all callers.
	 *
	 * Classes from optional plugins (e.g. GameplayAbilities) are intentionally
	 * excluded to avoid hard module dependencies. Their prefixes are resolved
	 * via GetOptionalPluginPrefixes() using class name string comparison.
	 *
	 * @return Const reference to the prefix map. Never invalid.
	 */
	static const TMap<UClass*, FString>& GetPrefixMap();

	/**
	 * Returns the prefix map for classes from optional plugins that cannot be
	 * referenced via StaticClass() without a hard module dependency.
	 *
	 * Keyed by native class name (FName). These names are stable across UE versions —
	 * Epic cannot rename them without breaking backward compatibility for thousands
	 * of existing projects.
	 *
	 * @return Const reference to the optional prefix map. Never invalid.
	 */
	static const TMap<FName, FString>& GetOptionalPluginPrefixes();

	/**
	 * Resolves the expected name prefix for a given asset.
	 *
	 * For Blueprint assets, walks the native parent class hierarchy
	 * (via UBlueprint::ParentClass) to correctly identify framework subclasses
	 * (e.g. GameMode, PlayerController) and optional-plugin classes
	 * (e.g. GameplayAbility, GameplayEffect) without requiring a hard module
	 * dependency on GameplayAbilities.
	 *
	 * For non-Blueprint assets, walks the asset's own class hierarchy.
	 *
	 * @param AssetClass  The class of the asset as returned by FAssetData::GetClass(). Must not be null.
	 * @param Asset       The asset UObject. Used to inspect UBlueprint::ParentClass when applicable.
	 *                    May be null for non-Blueprint assets.
	 * @return            Pointer to the registered prefix string, or nullptr if unknown.
	 */
	static const FString* FindPrefixForClass(UClass* AssetClass,
	                                         UObject* Asset);

	/**
	 * Resolves the expected prefix for a Blueprint asset without loading it into memory.
	 *
	 * Reads the "ParentClass" Asset Registry tag and walks the class hierarchy,
	 * checking both the main prefix map and the optional plugin map (GAS, etc.)
	 * by class name string. Falls back to "BP_" if the tag is absent or the
	 * parent class is not registered.
	 *
	 * @param AssetData  The asset metadata from the Asset Registry.
	 * @return           Pointer to the resolved prefix string. Never null.
	 */
	static const FString* ResolveBlueprintPrefixFromTag(const FAssetData& AssetData);

	/**
	 * Applies the correct prefix to a single asset using FindPrefixForClass().
	 * Handles UMaterialInstanceConstant stripping internally (removes "M_" and "_Inst"
	 * before applying "MI_").
	 *
	 * @param Asset     The asset to rename. Must be a valid non-null UObject.
	 * @return          The outcome of the rename attempt as EBertaRenameResult.
	 *
	 * @note Callers are responsible for logging the result — this function logs
	 *       only internal errors that indicate programming mistakes.
	 */
	static EBertaRenameResult RenameAssetWithPrefix(UObject* Asset);
};
