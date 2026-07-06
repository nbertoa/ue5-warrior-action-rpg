#include "BertaDevKit.h"

#include "Modules/ModuleManager.h"

void FBertaDevKitModule::StartupModule()
{
	// Nothing to initialize yet.
	// Future use: register custom asset types, console commands,
	// or global state needed by the plugin's subsystems.
}

void FBertaDevKitModule::ShutdownModule()
{
	// Nothing to clean up yet.
	// Future use: unregister anything registered in StartupModule.
}

// Registers this module with the Unreal Engine module system.
// The first argument must match the module Name in BertaDevKit.uplugin.
// The second argument is the implementation class.
IMPLEMENT_MODULE(FBertaDevKitModule,
                 BertaDevKit)
