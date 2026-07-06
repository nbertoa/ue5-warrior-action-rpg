#pragma once

/**
 * @file BertaEditorToolbar.h
 * @brief Registers and unregisters the BertaDevKit entry in the Level Editor Tools menu.
 */

/**
 * Manages the BertaDevKit entry in the Level Editor Tools menu.
 *
 * Lifetime is owned by FBertaDevKitEditorModule via TUniquePtr.
 * Register() must be called from an OnPostEngineInit delegate — not directly
 * from StartupModule() — because UToolMenus is not yet available at that point.
 * Unregister() must be called from ShutdownModule().
 *
 * @note This class is intentionally not a UObject. It has no need for GC,
 *       reflection, or Blueprint exposure — it is pure editor infrastructure.
 */
class BERTADEVKITEDITOR_API FBertaEditorToolbar
{
public:
	FBertaEditorToolbar() = default;
	~FBertaEditorToolbar() = default;

	// Non-copyable — ownership is exclusive via TUniquePtr.
	FBertaEditorToolbar(const FBertaEditorToolbar&) = delete;
	FBertaEditorToolbar& operator=(const FBertaEditorToolbar&) = delete;

	/**
	 * Registers the BertaDevKit entry under LevelEditor.MainMenu.Tools.
	 * Safe to call only after OnPostEngineInit has fired.
	 */
	void Register();

	/**
	 * Removes all menu entries registered under the BertaDevKit owner name.
	 * Must be called from FBertaDevKitEditorModule::ShutdownModule().
	 */
	void Unregister();

private:
	/**
	 * Callback executed when the user clicks "Run Asset Audit" in the Tools menu.
	 * Delegates directly to UBertaAssetAuditor::RunAudit().
	 */
	void OnRunAssetAuditClicked();

	/**
	 * Callback executed when the user clicks "Fix Asset Naming" in the Tools menu.
	 * Delegates directly to UBertaAssetAuditor::RunAuditAndFix().
	 */
	void OnFixAssetNamingClicked();

	/**
	 * Callback executed when the user clicks "Run World Validation" in the Tools menu.
	 * Delegates directly to UBertaWorldValidation::RunValidation().
	 */
	void OnRunWorldValidationClicked();
};
