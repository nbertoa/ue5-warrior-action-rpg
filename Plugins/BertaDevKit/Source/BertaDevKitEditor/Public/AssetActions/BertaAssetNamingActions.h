#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"

#include "BertaAssetNamingActions.generated.h"

/**
 * Exposes asset naming operations to the Content Browser right-click context menu.
 * All logic is delegated to UBertaAssetAuditor and UBertaAssetNamingUtils.
 */
UCLASS(Abstract)
class BERTADEVKITEDITOR_API UBertaAssetNamingActions : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	/**
	 * Scans assets in scope and reports naming violations to the Output Log.
	 * Does not modify any assets.
	 */
	UFUNCTION(CallInEditor)
	void AuditAssetNaming();

	/**
	 * Scans assets in scope and renames violators to match naming conventions.
	 */
	UFUNCTION(CallInEditor)
	void FixAssetNaming();
};
