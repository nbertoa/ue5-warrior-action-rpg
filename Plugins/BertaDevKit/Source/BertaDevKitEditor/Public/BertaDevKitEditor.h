#pragma once

#include "Modules/ModuleManager.h"

class FBertaEditorToolbar;

/**
 * Editor module for BertaDevKit.
 * Owns the lifetime of all editor-side subsystems, including the Tools menu toolbar.
 */
class FBertaDevKitEditorModule : public IModuleInterface
{
public:
	//~ Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface

private:
	/**
	 * Called once the engine and editor systems (including UToolMenus) are fully initialized.
	 * This is the correct point to register menu extensions — not StartupModule() directly.
	 */
	void OnPostEngineInit();

	/** Owns the Tools menu entry for BertaDevKit. Null before StartupModule, valid after. */
	TUniquePtr<FBertaEditorToolbar> EditorToolbar;
};
