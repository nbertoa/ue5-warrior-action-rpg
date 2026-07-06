// WarriorRPG.Build.cs
// Build configuration for the WarriorRPG module.
// Defines all module dependencies required for compilation and linking.

using UnrealBuildTool;

public class WarriorRPG : ModuleRules
{
	public WarriorRPG(ReadOnlyTargetRules Target) : base(Target)
	{
		// Use explicit or shared precompiled headers for faster compilation.
		// This is the recommended setting for UE5 projects — it avoids pulling in
		// the monolithic shared PCH and keeps compile times predictable as the project grows.
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public dependencies are visible to other modules that depend on WarriorRPG.
		// Keep this list to the minimum needed — over-exposing modules increases compile coupling.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore", // Legacy input types still referenced by some engine APIs alongside Enhanced Input
			"EnhancedInput", // UE5 input system — replaces the legacy axis/action bindings
			"UMG", // Base UI framework — required even when using Common UI
			"CommonUI", // Common UI plugin — provides CommonActivatableWidget, input routing, etc.
			"CommonInput", // Common Input plugin — provides platform-aware input type detection
			"Slate", // Low-level UI framework — needed for custom Slate widgets or SNew() usage
			"SlateCore", // Core Slate types and rendering
			"GameplayTags", // Hierarchical tag system — used to identify inputs, abilities, states, etc.
			"GameplayAbilities", // Gameplay Ability System — abilities, effects, attribute sets, and the ASC
			"GameplayTasks", // Async task framework — required by GAS for ability tasks (e.g., WaitTargetData)
			"DeveloperSettings", // UDeveloperSettings base class — required by UWarriorUISettings
			"AIModule", // AI framework — AIController, perception system, behavior trees
			"AnimGraphRuntime", // Animation graph nodes at runtime — required for KismetAnimationLibrary
			"MotionWarping", // Root motion warping — aligns attack/roll animations to target locations
			"Niagara", // VFX system — required by AWarriorProjectileBase's UNiagaraComponent (public header)
			"NavigationSystem",
		});

		// Private dependencies are only visible within this module's .cpp files.
		// Move modules here when they are implementation details not exposed in public headers.
		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}