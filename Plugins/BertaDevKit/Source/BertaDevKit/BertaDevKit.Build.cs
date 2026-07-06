using UnrealBuildTool;

public class BertaDevKit : ModuleRules
{
	public BertaDevKit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public dependencies are visible to both this module
		// and any module that depends on BertaDevKit.
		// Keep this list minimal — every entry here becomes
		// a transitive dependency for all consumers.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"DeveloperSettings" // UDeveloperSettings
		});

		// Private dependencies are only visible inside BertaDevKit.
		// Consumers of the plugin cannot see these — good for
		// implementation details that shouldn't leak into the public API.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// Empty for now. Add here as the plugin grows.
			// Examples:
			// "GameplayAbilities" — when GAS helpers are added
			// "UMG"              — when widget utilities are added
			// "EnhancedInput"    — when input helpers are added
		});
	}
}