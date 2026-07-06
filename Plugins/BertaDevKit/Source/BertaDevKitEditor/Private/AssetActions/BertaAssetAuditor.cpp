#include "AssetActions/BertaAssetAuditor.h"

#include "AssetActions/BertaAssetNamingUtils.h"
#include "Log/BertaDevKitEditorLog.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorUtilityLibrary.h"
#include "Engine/Blueprint.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Widgets/Notifications/SNotificationList.h"

// ----------------------------------------------------------------
// Internal helpers
// ----------------------------------------------------------------

namespace
{
	/** Builds and displays an FNotificationInfo toast with the given message. */
	void ShowNotification(const FText& Message,
	                      const SNotificationItem::ECompletionState State)
	{
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.ExpireDuration = 4.0f;

		TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);

		if (Notification.IsValid())
		{
			Notification->SetCompletionState(State);
		}
	}
}

// ----------------------------------------------------------------
// ResolveAssetScope
// ----------------------------------------------------------------

void UBertaAssetAuditor::ResolveAssetScope(TArray<FAssetData>& OutAssets)
{
	OutAssets.Reset();

	// Priority 1: individually selected assets in the Content Browser.
	const TArray<FAssetData> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssetData();

	if (!SelectedAssets.IsEmpty())
	{
		OutAssets = SelectedAssets;

		UE_LOG(LogBertaDevKitEditor,
		       Log,
		       TEXT("[UBertaAssetAuditor::ResolveAssetScope] Using %d selected asset(s)."),
		       OutAssets.Num());

		return;
	}

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;

	// Priority 2: the folder active in the Content Browser directory tree.
	FString ActiveFolderPath;
	const bool bHasActivePath = UEditorUtilityLibrary::GetCurrentContentBrowserPath(ActiveFolderPath);

	if (bHasActivePath && !ActiveFolderPath.IsEmpty())
	{
		Filter.PackagePaths.Add(FName(*ActiveFolderPath));

		UE_LOG(LogBertaDevKitEditor,
		       Log,
		       TEXT("[UBertaAssetAuditor::ResolveAssetScope] Scanning active folder: %s"),
		       *ActiveFolderPath);
	}
	else
	{
		// Priority 3: full /Game/ scan as last resort.
		Filter.PackagePaths.Add(FName(TEXT("/Game")));

		UE_LOG(LogBertaDevKitEditor,
		       Log,
		       TEXT("[UBertaAssetAuditor::ResolveAssetScope] No folder active — falling back to full /Game/ scan."));
	}

	AssetRegistryModule.Get().GetAssets(Filter,
	                                    OutAssets);

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::ResolveAssetScope] Found %d asset(s) in scope."),
	       OutAssets.Num());
}

// ----------------------------------------------------------------
// AuditAssetNaming
// ----------------------------------------------------------------

void UBertaAssetAuditor::AuditAssetNaming()
{
	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::AuditAssetNaming] Starting audit..."));

	TArray<FAssetData> Assets;
	ResolveAssetScope(Assets);

	int32 ViolationCount = 0;

	for (const FAssetData& AssetData : Assets)
	{
		UClass* AssetClass = AssetData.GetClass();

		if (!AssetClass)
		{
			// Class not loaded — skip silently, not a violation.
			continue;
		}

		// For Blueprint assets, resolve the prefix via the Asset Registry tag to
		// avoid loading the asset into memory during a dry audit.
		const FString* FoundPrefix = nullptr;

		if (AssetClass == UBlueprint::StaticClass() || AssetClass->IsChildOf(UBlueprint::StaticClass()))
		{
			FoundPrefix = UBertaAssetNamingUtils::ResolveBlueprintPrefixFromTag(AssetData);
		}
		else
		{
			FoundPrefix = UBertaAssetNamingUtils::FindPrefixForClass(AssetClass,
			                                                         nullptr);
		}

		if (!FoundPrefix || FoundPrefix->IsEmpty())
		{
			UE_LOG(LogBertaDevKitEditor,
			       Verbose,
			       TEXT("[UBertaAssetAuditor::AuditAssetNaming] Unknown class, skipping: %s (%s)"),
			       *AssetData.AssetName.ToString(),
			       *AssetData.AssetClassPath.GetAssetName().ToString());
			continue;
		}

		const FString AssetName = AssetData.AssetName.ToString();

		if (!AssetName.StartsWith(*FoundPrefix))
		{
			UE_LOG(LogBertaDevKitEditor,
			       Warning,
			       TEXT(
				       "[UBertaAssetAuditor::AuditAssetNaming] VIOLATION — Asset: %s | Class: %s | Expected prefix: %s | Path: %s"
			       ),
			       *AssetName,
			       *AssetClass->GetName(),
			       **FoundPrefix,
			       *AssetData.PackagePath.ToString());

			++ViolationCount;
		}
	}

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::AuditAssetNaming] Audit complete — %d violation(s) found."),
	       ViolationCount);

	const FText NotificationText = ViolationCount > 0
		                               ? FText::Format(NSLOCTEXT("BertaDevKit",
		                                                         "AuditViolations",
		                                                         "Asset Audit: {0} violation(s) found. See Output Log."),
		                                               FText::AsNumber(ViolationCount))
		                               : NSLOCTEXT("BertaDevKit",
		                                           "AuditClean",
		                                           "Asset Audit: No violations found.");

	const SNotificationItem::ECompletionState NotificationState = ViolationCount > 0
		                                                              ? SNotificationItem::CS_Fail
		                                                              : SNotificationItem::CS_Success;

	ShowNotification(NotificationText,
	                 NotificationState);
}

// ----------------------------------------------------------------
// FixAssetNaming
// ----------------------------------------------------------------

void UBertaAssetAuditor::FixAssetNaming()
{
	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::FixAssetNaming] Starting fix..."));

	TArray<FAssetData> Assets;
	ResolveAssetScope(Assets);

	int32 RenamedCount = 0;
	int32 SkippedCount = 0;

	for (const FAssetData& AssetData : Assets)
	{
		UClass* AssetClass = AssetData.GetClass();

		if (!AssetClass)
		{
			continue;
		}

		// Load the asset first — FindPrefixForClass needs the UBlueprint object
		// to inspect ParentClass for framework and optional-plugin Blueprints.
		UObject* const LoadedAsset = AssetData.GetAsset();

		if (!IsValid(LoadedAsset))
		{
			UE_LOG(LogBertaDevKitEditor,
			       Warning,
			       TEXT("[UBertaAssetAuditor::FixAssetNaming] Failed to load asset: %s"),
			       *AssetData.AssetName.ToString());
			++SkippedCount;
			continue;
		}

		// Use FindPrefixForClass for loaded Blueprint assets so the ParentClass walk
		// is used — same resolution path as AuditAssetNaming. For generic Blueprints
		// where the ParentClass walk finds no match, fall back to the tag-based resolver.
		const FString* FoundPrefix = nullptr;

		if (AssetClass == UBlueprint::StaticClass() || AssetClass->IsChildOf(UBlueprint::StaticClass()))
		{
			FoundPrefix = UBertaAssetNamingUtils::FindPrefixForClass(AssetClass,
			                                                         LoadedAsset);

			if (!FoundPrefix)
			{
				FoundPrefix = UBertaAssetNamingUtils::ResolveBlueprintPrefixFromTag(AssetData);
			}
		}
		else
		{
			FoundPrefix = UBertaAssetNamingUtils::FindPrefixForClass(AssetClass,
			                                                         nullptr);
		}

		if (!FoundPrefix || FoundPrefix->IsEmpty())
		{
			UE_LOG(LogBertaDevKitEditor,
			       Verbose,
			       TEXT("[UBertaAssetAuditor::FixAssetNaming] Unknown class, skipping: %s"),
			       *AssetData.AssetName.ToString());
			continue;
		}

		// Asset already correct — nothing to fix.
		if (AssetData.AssetName.ToString().StartsWith(*FoundPrefix))
		{
			continue;
		}

		// Rename using the resolved prefix directly — bypasses RenameAssetWithPrefix's
		// own class walk, which does not have access to ParentClass context.
		FString CleanName = AssetData.AssetName.ToString();

		// Material instances auto-named by the engine may carry "M_" and "_Inst".
		if (LoadedAsset->IsA<UMaterialInstanceConstant>())
		{
			CleanName.RemoveFromStart(TEXT("M_"));
			CleanName.RemoveFromEnd(TEXT("_Inst"));
		}

		// Animation montages may carry a "_Montage" suffix — strip before applying "AM_".
		if (LoadedAsset->IsA<UAnimMontage>())
		{
			CleanName.RemoveFromEnd(TEXT("_Montage"));
		}

		const FString NewName = *FoundPrefix + CleanName;
		UEditorUtilityLibrary::RenameAsset(LoadedAsset,
		                                   NewName);

		UE_LOG(LogBertaDevKitEditor,
		       Log,
		       TEXT("[UBertaAssetAuditor::FixAssetNaming] Fixed: %s → %s"),
		       *AssetData.AssetName.ToString(),
		       *NewName);

		++RenamedCount;
	}

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetAuditor::FixAssetNaming] Done — renamed %d asset(s), skipped %d."),
	       RenamedCount,
	       SkippedCount);

	const FText NotificationText = FText::Format(NSLOCTEXT("BertaDevKit",
	                                                       "AuditFixDone",
	                                                       "Asset Fix: {0} renamed, {1} skipped. See Output Log."),
	                                             FText::AsNumber(RenamedCount),
	                                             FText::AsNumber(SkippedCount));

	const SNotificationItem::ECompletionState NotificationState = SkippedCount > 0
		                                                              ? SNotificationItem::CS_Fail
		                                                              : SNotificationItem::CS_Success;

	ShowNotification(NotificationText,
	                 NotificationState);
}
