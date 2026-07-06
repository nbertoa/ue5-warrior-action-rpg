#include "AssetActions/BertaAssetNamingActions.h"

#include "AssetActions/BertaAssetAuditor.h"
#include "Log/BertaDevKitEditorLog.h"

// ----------------------------------------------------------------
// AuditAssetNaming
// ----------------------------------------------------------------

void UBertaAssetNamingActions::AuditAssetNaming()
{
	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetNamingActions::AuditAssetNaming] Triggered from Content Browser context menu."));

	UBertaAssetAuditor::AuditAssetNaming();
}

// ----------------------------------------------------------------
// FixAssetNaming
// ----------------------------------------------------------------

void UBertaAssetNamingActions::FixAssetNaming()
{
	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[UBertaAssetNamingActions::FixAssetNaming] Triggered from Content Browser context menu."));

	UBertaAssetAuditor::FixAssetNaming();
}
