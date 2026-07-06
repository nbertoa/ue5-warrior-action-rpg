#include "BertaDevKitEditor.h"

#include "Toolbar/BertaEditorToolbar.h"
#include "Log/BertaDevKitEditorLog.h"

#include "Misc/CoreDelegates.h"

void FBertaDevKitEditorModule::StartupModule()
{
	// Construct the toolbar object immediately so it is ready when the delegate fires.
	// Registration itself must wait for OnPostEngineInit because UToolMenus
	// is not guaranteed to exist at this point in the startup sequence.
	EditorToolbar = MakeUnique<FBertaEditorToolbar>();

	// Bind to OnPostEngineInit using a named member callback — avoids a raw lambda
	// and makes the call traceable in the debugger.
	FCoreDelegates::OnPostEngineInit.AddRaw(this,
	                                        &FBertaDevKitEditorModule::OnPostEngineInit);

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT(
		       "[FBertaDevKitEditorModule::StartupModule] Module started. Toolbar registration deferred to OnPostEngineInit."
	       ));
}

void FBertaDevKitEditorModule::ShutdownModule()
{
	// Remove the delegate binding before the toolbar is destroyed.
	// If OnPostEngineInit has not fired yet (edge case on very fast shutdown),
	// this prevents a dangling callback.
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);

	if (EditorToolbar)
	{
		EditorToolbar->Unregister();
		EditorToolbar.Reset();
	}

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[FBertaDevKitEditorModule::ShutdownModule] Module shut down. Toolbar unregistered."));
}

void FBertaDevKitEditorModule::OnPostEngineInit()
{
	// This delegate fires once — no need to unbind after registration.
	// The engine guarantees it is not called again after this point.
	if (EditorToolbar)
	{
		EditorToolbar->Register();
	}
}

IMPLEMENT_MODULE(FBertaDevKitEditorModule,
                 BertaDevKitEditor)
