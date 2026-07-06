#pragma once

#include "UObject/Object.h"

#include "BertaAssetAuditor.generated.h"

/**
 * Scans assets in the project and reports or fixes naming convention violations.
 *
 * Scope resolution priority:
 *   1. Individually selected assets in the Content Browser.
 *   2. The folder currently active in the Content Browser directory tree.
 *   3. Full recursive scan under /Game/ as a last resort.
 *
 * All prefix resolution is delegated to UBertaAssetNamingUtils.
 *
 * @note This class is never instantiated — all entry points are static.
 */
UCLASS()
class BERTADEVKITEDITOR_API UBertaAssetAuditor : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Scans assets and reports naming violations to LogBertaDevKitEditor.
	 * Displays an FNotificationInfo with the total violation count on completion.
	 * Does not modify any assets.
	 */
	static void AuditAssetNaming();

	/**
	 * Scans assets for naming violations and renames each violator.
	 * Reports renamed and skipped counts via LogBertaDevKitEditor and FNotificationInfo.
	 */
	static void FixAssetNaming();

private:
	/**
	 * Collects the asset scope using the following priority:
	 *   1. Selected assets — returned immediately if any are selected.
	 *   2. Active Content Browser folder — scanned recursively via Asset Registry.
	 *   3. Full /Game/ scan — used only when neither of the above yields a scope.
	 *
	 * @param OutAssets  Populated with the resolved asset scope.
	 */
	static void ResolveAssetScope(TArray<FAssetData>& OutAssets);
};
