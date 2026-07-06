#include "Toolbar/BertaEditorToolbar.h"

#include "AssetActions/BertaAssetAuditor.h"
#include "Log/BertaDevKitEditorLog.h"
#include "WorldValidation/BertaWorldValidation.h"

#include "ToolMenus.h"
#include "ToolMenuEntry.h"
#include "ToolMenuSection.h"

// ─── Constants ───────────────────────────────────────────────────────────────
// Defined here (not in the header) so each translation unit gets exactly one
// copy. Declaring them as 'static const' in a header would produce a separate
// copy per translation unit, which is wasteful for non-trivial types like FText.

namespace
{
	/** Owner name used to identify all menu entries registered by this toolbar.
	 *  Assigned to Entry.Owner on each FToolMenuEntry so UnregisterOwner can locate them. */
	const FName BertaOwnerName = TEXT("BertaDevKit");

	/** Display label and tooltip for the "Run Asset Audit" menu entry. */
	const FText RunAssetAuditLabel = NSLOCTEXT("BertaDevKit",
	                                           "RunAssetAudit",
	                                           "Run Asset Audit");
	const FText RunAssetAuditTooltip = NSLOCTEXT("BertaDevKit",
	                                             "RunAssetAuditTooltip",
	                                             "Audit all assets in /Game/ and report naming convention violations.");

	/** Display label and tooltip for the "Fix Asset Naming" menu entry. */
	const FText FixAssetNamingLabel = NSLOCTEXT("BertaDevKit",
	                                            "FixAssetNaming",
	                                            "Fix Asset Naming");
	const FText FixAssetNamingTooltip = NSLOCTEXT("BertaDevKit",
	                                              "FixAssetNamingTooltip",
	                                              "Rename all assets under /Game/ (or current selection) to match naming conventions.");

	/** Display label and tooltip for the "Run World Validation" menu entry. */
	const FText WorldValidationLabel = NSLOCTEXT("BertaDevKit",
	                                             "RunWorldValidation",
	                                             "Run World Validation");
	const FText WorldValidationTooltip = NSLOCTEXT("BertaDevKit",
	                                               "RunWorldValidationTooltip",
	                                               "Validate all actors in the open level and report violations to the Output Log.");
}

// ─── Register / Unregister ───────────────────────────────────────────────────

void FBertaEditorToolbar::Register()
{
	if (!UToolMenus::IsToolMenuUIEnabled())
	{
		UE_LOG(LogBertaDevKitEditor,
		       Warning,
		       TEXT("[FBertaEditorToolbar::Register] UToolMenus is not available yet."));
		return;
	}

	UToolMenu* const ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	if (!ToolsMenu)
	{
		UE_LOG(LogBertaDevKitEditor,
		       Error,
		       TEXT("[FBertaEditorToolbar::Register] Failed to extend 'LevelEditor.MainMenu.Tools'."));
		return;
	}

	FToolMenuSection& Section = ToolsMenu->AddSection("BertaDevKitSection",
	                                                  NSLOCTEXT("BertaDevKit",
	                                                            "BertaDevKitSection",
	                                                            "BertaDevKit"));

	// Each entry's Owner is set to BertaOwnerName so that UnregisterOwner("BertaDevKit")
	// in Unregister() can locate and remove them. This is the correct mechanism —
	// UToolMenus has no RegisterOwner() API; ownership is established per-entry.

	// "Run Asset Audit" — reports violations without modifying assets.
	{
		FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry("BertaRunAssetAudit",
		                                                     RunAssetAuditLabel,
		                                                     RunAssetAuditTooltip,
		                                                     FSlateIcon(),
		                                                     FUIAction(FExecuteAction::CreateRaw(this,
			                                                     &FBertaEditorToolbar::OnRunAssetAuditClicked)));
		Entry.Owner = FToolMenuOwner(BertaOwnerName);
		Section.AddEntry(Entry);
	}

	// "Fix Asset Naming" — renames violators to match naming conventions.
	{
		FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry("BertaFixAssetNaming",
		                                                     FixAssetNamingLabel,
		                                                     FixAssetNamingTooltip,
		                                                     FSlateIcon(),
		                                                     FUIAction(FExecuteAction::CreateRaw(this,
			                                                     &FBertaEditorToolbar::OnFixAssetNamingClicked)));
		Entry.Owner = FToolMenuOwner(BertaOwnerName);
		Section.AddEntry(Entry);
	}

	// "Run World Validation" — iterates all actors in the open level and reports violations.
	{
		FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry("BertaRunWorldValidation",
		                                                     WorldValidationLabel,
		                                                     WorldValidationTooltip,
		                                                     FSlateIcon(),
		                                                     FUIAction(FExecuteAction::CreateRaw(this,
			                                                     &FBertaEditorToolbar::OnRunWorldValidationClicked)));
		Entry.Owner = FToolMenuOwner(BertaOwnerName);
		Section.AddEntry(Entry);
	}

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[FBertaEditorToolbar::Register] BertaDevKit Tools menu entries registered."));
}

void FBertaEditorToolbar::Unregister()
{
	if (!UToolMenus::IsToolMenuUIEnabled())
	{
		return;
	}

	// UnregisterOwner removes all FToolMenuEntry instances whose Owner field
	// was set to BertaOwnerName. This cleanly handles hot-reload and plugin unload.
	UToolMenus::Get()->UnregisterOwner(BertaOwnerName);

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[FBertaEditorToolbar::Unregister] BertaDevKit Tools menu entries removed."));
}

// ─── Callbacks ───────────────────────────────────────────────────────────────

void FBertaEditorToolbar::OnRunAssetAuditClicked()
{
	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[FBertaEditorToolbar::OnRunAssetAuditClicked] User triggered Run Asset Audit."));

	UBertaAssetAuditor::AuditAssetNaming();
}

void FBertaEditorToolbar::OnFixAssetNamingClicked()
{
	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[FBertaEditorToolbar::OnFixAssetNamingClicked] User triggered Fix Asset Naming."));

	UBertaAssetAuditor::FixAssetNaming();
}

void FBertaEditorToolbar::OnRunWorldValidationClicked()
{
	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[FBertaEditorToolbar::OnRunWorldValidationClicked] User triggered Run World Validation."));

	UBertaWorldValidation::RunValidation();
}
