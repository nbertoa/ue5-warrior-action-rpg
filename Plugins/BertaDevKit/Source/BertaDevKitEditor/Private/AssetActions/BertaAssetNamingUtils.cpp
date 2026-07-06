#include "AssetActions/BertaAssetNamingUtils.h"

#include "Log/BertaDevKitEditorLog.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EditorUtilityLibrary.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AimOffsetBlendSpace1D.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Blueprint.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/UserDefinedEnum.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialParameterCollection.h"
#include "NiagaraEmitter.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "StructUtils/UserDefinedStruct.h"

// ----------------------------------------------------------------
// Internal helpers
// ----------------------------------------------------------------

namespace
{
	/**
	 * Extracts the native class name from an Asset Registry "ParentClass" tag value.
	 *
	 * Expected format: /Script/CoreUObject.Class'/Script/Module.ClassName'
	 * Returns the ClassName portion after the last '.', or NAME_None if the
	 * format is unexpected.
	 */
	FName ExtractClassNameFromTag(const FString& TagValue)
	{
		// Find the last '.' — the class name follows it.
		int32 DotIndex = INDEX_NONE;
		if (!TagValue.FindLastChar(TEXT('.'),
		                           DotIndex))
		{
			return NAME_None;
		}

		// Extract everything after the last '.' and strip the closing '\''.
		FString ClassName = TagValue.RightChop(DotIndex + 1);
		ClassName.RemoveFromEnd(TEXT("'"));

		if (ClassName.IsEmpty())
		{
			return NAME_None;
		}

		return FName(*ClassName);
	}
}

// ----------------------------------------------------------------
// GetPrefixMap
// ----------------------------------------------------------------

const TMap<UClass*, FString>& UBertaAssetNamingUtils::GetPrefixMap()
{
	// Static local — constructed once on first call, reused for the lifetime
	// of the editor session. No locking needed: asset operations are single-threaded.
	//
	// Classes from optional plugins (GameplayAbilities, etc.) are intentionally
	// absent. Their prefixes are resolved via GetOptionalPluginPrefixes() using
	// class name string comparison, avoiding hard module dependencies that would
	// prevent the editor from launching on projects without those plugins.
	static const TMap<UClass*, FString> PrefixMap = {
		// Blueprints — specific subclasses must appear before UBlueprint so that
		// the hierarchy walk in FindPrefixForClass matches the most derived type first.
		{UAnimBlueprint::StaticClass(), TEXT("ABP_")}, {UUserWidget::StaticClass(), TEXT("WBP_")},
		{UBlueprint::StaticClass(), TEXT("BP_")},

		// Meshes
		{UStaticMesh::StaticClass(), TEXT("SM_")}, {USkeletalMesh::StaticClass(), TEXT("SKM_")},

		// Materials
		{UMaterial::StaticClass(), TEXT("M_")}, {UMaterialInstanceConstant::StaticClass(), TEXT("MI_")},
		{UMaterialParameterCollection::StaticClass(), TEXT("MPC_")},

		// Textures
		{UTexture2D::StaticClass(), TEXT("T_")}, {UTextureCube::StaticClass(), TEXT("T_")},
		{UTextureRenderTarget2D::StaticClass(), TEXT("RT_")},

		// Animation — AimOffset is a BlendSpace subclass; must appear before BlendSpace.
		{UAimOffsetBlendSpace::StaticClass(), TEXT("AO_")}, {UAimOffsetBlendSpace1D::StaticClass(), TEXT("AO_")},
		{UAnimSequence::StaticClass(), TEXT("AS_")}, {UAnimMontage::StaticClass(), TEXT("AM_")},
		{UBlendSpace::StaticClass(), TEXT("BS_")}, {UBlendSpace1D::StaticClass(), TEXT("BS_")},

		// VFX
		{UParticleSystem::StaticClass(), TEXT("PS_")}, {UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")},

		// Audio
		{USoundWave::StaticClass(), TEXT("SW_")}, {USoundCue::StaticClass(), TEXT("SC_")},

		// Data
		{UDataTable::StaticClass(), TEXT("DT_")}, {UDataAsset::StaticClass(), TEXT("DA_")},

		// Physics
		{UPhysicsAsset::StaticClass(), TEXT("PA_")},

		// User-defined types
		{UUserDefinedEnum::StaticClass(), TEXT("E_")}, {UUserDefinedStruct::StaticClass(), TEXT("F_")},

		// Framework Blueprint parent classes — resolved via UBlueprint::ParentClass walk.
		{AGameModeBase::StaticClass(), TEXT("GM_")}, {AGameMode::StaticClass(), TEXT("GM_")},
		{APlayerController::StaticClass(), TEXT("PC_")}, {ACharacter::StaticClass(), TEXT("CH_")},
		{APawn::StaticClass(), TEXT("P_")},

		// AI
		{UBlackboardData::StaticClass(), TEXT("BB_")}, {AAIController::StaticClass(), TEXT("AIC_")},
		{UBTDecorator::StaticClass(), TEXT("BTD_")}, {UBTService::StaticClass(), TEXT("BTS_")},
		{UBTTaskNode::StaticClass(), TEXT("BTT_")},

		// EQS
		{UEnvQuery::StaticClass(), TEXT("EQS_")}, {UEnvQueryContext::StaticClass(), TEXT("EQSC_")},

		// Input
		{UInputAction::StaticClass(), TEXT("IA_")}, {UInputMappingContext::StaticClass(), TEXT("IMC_")},
	};

	return PrefixMap;
}

// ----------------------------------------------------------------
// GetOptionalPluginPrefixes
// ----------------------------------------------------------------

const TMap<FName, FString>& UBertaAssetNamingUtils::GetOptionalPluginPrefixes()
{
	// Static local — keyed by native class name string to avoid hard module
	// dependencies on optional plugins (e.g. GameplayAbilities).
	// These names are stable across UE versions — Epic cannot rename them
	// without breaking backward compatibility for thousands of projects.
	static const TMap<FName, FString> OptionalPrefixes = {
		// Gameplay Ability System
		{FName(TEXT("GameplayAbility")), TEXT("GA_")}, {FName(TEXT("GameplayEffect")), TEXT("GE_")},
		{FName(TEXT("GameplayCueNotify_Static")), TEXT("GC_")}, {FName(TEXT("GameplayCueNotify_Actor")), TEXT("GC_")},
		{FName(TEXT("GameplayAbilityBlueprint")), TEXT("GA_")}, {FName(TEXT("GameplayEffectBlueprint")), TEXT("GE_")},
	};

	return OptionalPrefixes;
}

// ----------------------------------------------------------------
// FindPrefixForClass
// ----------------------------------------------------------------

const FString* UBertaAssetNamingUtils::FindPrefixForClass(UClass* AssetClass,
                                                          UObject* Asset)
{
	const TMap<UClass*, FString>& PrefixMap = GetPrefixMap();
	const TMap<FName, FString>& OptionalPrefixes = GetOptionalPluginPrefixes();

	// Path A: asset is loaded and is a Blueprint — walk UBlueprint::ParentClass directly.
	// The meaningful class for prefix resolution is the native parent, not the asset
	// class itself (which is always UBlueprint or a subclass like UAnimBlueprint).
	if (const UBlueprint* const BP = Cast<UBlueprint>(Asset))
	{
		UClass* ParentClass = BP->ParentClass;

		while (ParentClass)
		{
			if (const FString* Found = PrefixMap.Find(ParentClass))
			{
				return Found;
			}

			if (const FString* Found = OptionalPrefixes.Find(ParentClass->GetFName()))
			{
				return Found;
			}

			ParentClass = ParentClass->GetSuperClass();
		}

		// Blueprint with no registered parent — unknown.
		return nullptr;
	}

	// Path B: non-Blueprint asset — walk the asset's own class hierarchy directly.
	UClass* CurrentClass = AssetClass;

	while (CurrentClass)
	{
		if (const FString* Found = PrefixMap.Find(CurrentClass))
		{
			return Found;
		}

		CurrentClass = CurrentClass->GetSuperClass();
	}

	return nullptr;
}

// ----------------------------------------------------------------
// ResolveBlueprintPrefixFromTag
// ----------------------------------------------------------------

const FString* UBertaAssetNamingUtils::ResolveBlueprintPrefixFromTag(const FAssetData& AssetData)
{
	const TMap<UClass*, FString>& PrefixMap = GetPrefixMap();
	const TMap<FName, FString>& OptionalPrefixes = GetOptionalPluginPrefixes();

	// If the asset class itself has a direct entry in the prefix map
	// (e.g. UAnimBlueprint → "ABP_"), use it immediately without parsing the tag.
	// This avoids misrouting AnimBlueprints through the ParentClass walk,
	// which would resolve to the animation class rather than UAnimBlueprint.
	UClass* const AssetClass = AssetData.GetClass();
	if (AssetClass)
	{
		// For UBlueprint specifically, skip the direct map lookup and proceed to the
		// ParentClass tag walk — the meaningful prefix lives in the native parent hierarchy,
		// not in the asset class itself.
		// Specific Blueprint subclasses (UAnimBlueprint, UGameplayAbilityBlueprint, etc.)
		// are still resolved here via the direct lookup.
		const bool bIsGenericBlueprint = (AssetClass == UBlueprint::StaticClass());

		if (!bIsGenericBlueprint)
		{
			if (const FString* Found = PrefixMap.Find(AssetClass))
			{
				return Found;
			}

			if (const FString* Found = OptionalPrefixes.Find(AssetClass->GetFName()))
			{
				return Found;
			}
		}
	}

	// Read the "ParentClass" tag stored by the Asset Registry.
	// This tag is populated for all Blueprint assets without loading them into memory.
	// Format: /Script/CoreUObject.Class'/Script/Module.ClassName'
	FString ParentClassTag;
	AssetData.GetTagValue(TEXT("ParentClass"),
	                      ParentClassTag);

	if (ParentClassTag.IsEmpty())
	{
		// No tag — fall back to generic BP_ prefix.
		return PrefixMap.Find(UBlueprint::StaticClass());
	}

	const FName ParentClassName = ExtractClassNameFromTag(ParentClassTag);

	if (ParentClassName == NAME_None)
	{
		return PrefixMap.Find(UBlueprint::StaticClass());
	}

	// Check the optional plugin map first (GAS, etc.) — keyed by class name string.
	if (const FString* Found = OptionalPrefixes.Find(ParentClassName))
	{
		return Found;
	}

	// Try to resolve the actual UClass and walk the main prefix map.
	// This covers framework classes (AGameModeBase, APlayerController, etc.)
	// that are always available without optional plugins.
	if (UClass* const ParentClass = FindFirstObject<UClass>(*ParentClassName.ToString(),
	                                                        EFindFirstObjectOptions::None))
	{
		UClass* Current = ParentClass;

		while (Current)
		{
			if (const FString* Found = PrefixMap.Find(Current))
			{
				return Found;
			}

			Current = Current->GetSuperClass();
		}
	}

	// Parent class not registered — fall back to generic BP_ prefix.
	return PrefixMap.Find(UBlueprint::StaticClass());
}

// ----------------------------------------------------------------
// RenameAssetWithPrefix
// ----------------------------------------------------------------

EBertaRenameResult UBertaAssetNamingUtils::RenameAssetWithPrefix(UObject* const Asset)
{
	// Programming invariant — callers must validate before passing.
	if (!ensureMsgf(IsValid(Asset),
	                TEXT("UBertaAssetNamingUtils::RenameAssetWithPrefix — received null or invalid asset.")))
	{
		return EBertaRenameResult::UnknownClass;
	}

	// Resolve prefix using the full Blueprint-aware path.
	const FString* FoundPrefix = FindPrefixForClass(Asset->GetClass(),
	                                                Asset);

	if (!FoundPrefix || FoundPrefix->IsEmpty())
	{
		return EBertaRenameResult::UnknownClass;
	}

	// Asset already carries the correct prefix — nothing to do.
	if (Asset->GetName().StartsWith(*FoundPrefix))
	{
		return EBertaRenameResult::AlreadyCorrect;
	}

	// Take a mutable copy of the name to apply stripping and prefixing.
	FString CleanName = Asset->GetName();

	// Material instances auto-named by the engine may carry "M_" and "_Inst".
	// Strip both before applying "MI_" to produce a clean final name.
	if (Asset->IsA<UMaterialInstanceConstant>())
	{
		CleanName.RemoveFromStart(TEXT("M_"));
		CleanName.RemoveFromEnd(TEXT("_Inst"));
	}

	// Animation montages auto-named by the engine or imported may carry a "_Montage" suffix.
	// Strip it before applying "AM_" to produce a clean final name.
	if (Asset->IsA<UAnimMontage>())
	{
		CleanName.RemoveFromEnd(TEXT("_Montage"));
	}

	const FString NewName = *FoundPrefix + CleanName;
	UEditorUtilityLibrary::RenameAsset(Asset,
	                                   NewName);

	return EBertaRenameResult::Renamed;
}
