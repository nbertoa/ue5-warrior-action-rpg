#pragma once

#include "Modules/ModuleManager.h"

/**
 * BertaDevKit module interface.
 *
 * Entry point for the BertaDevKit plugin. Handles module lifecycle
 * (startup and shutdown) as required by the Unreal Engine module system.
 * Utility functionality lives in the individual subsystem headers
 * (e.g., Debug/BertaDebugUtils.h), not here.
 */
class FBertaDevKitModule : public IModuleInterface
{
public:
	//~ Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface
};
