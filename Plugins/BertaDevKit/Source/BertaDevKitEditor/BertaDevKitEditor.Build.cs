using UnrealBuildTool;

public class BertaDevKitEditor : ModuleRules
{
	public BertaDevKitEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AssetTools", // IAssetTools, FAssetToolsModule
			"Blutility", // UAssetActionUtility
			"BertaDevKit",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd", // UEditorUtilityLibrary, UEditorAssetLibrary
			"EditorScriptingUtilities", // UEditorAssetLibrary
			"Niagara", // UNiagaraSystem, UNiagaraEmitter
			"UMG", // UUserWidget
			"AIModule", // UBehaviorTree, UBlackboardData, UEnvQuery
			"Foliage", // UFoliageType
			"Landscape", // ULandscapeGrassType
			"PhysicsCore", // UPhysicalMaterial
			"Slate", // FSlateNotificationManager
			"SlateCore", // FNotificationInfo, FNotificationEntry
			"ToolMenus", // UToolMenus, FToolMenuEntry
			"WorkspaceMenuStructure", // WorkspaceMenu — tab spawner category registration
			"StructUtils", // UUserDefinedStruct (UE 5.6+)
			"EnhancedInput",
		});
	}
}