// WarriorBaseCharacter.h
// Base character class shared by all character types in WarriorRPG (hero, enemies, NPCs).
// Place shared functionality here: health, combat, animation, movement tuning.
// Implements IAbilitySystemInterface to integrate with the Gameplay Ability System.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Interfaces/PawnCombatInterface.h"
#include "Interfaces/PawnUIInterface.h"
#include "WarriorBaseCharacter.generated.h"

class UDataAsset_StartupDataBase;
class UMotionWarpingComponent;
class UWarriorAbilitySystemComponent;
class UWarriorAttributeSet;

/**
 * Abstract base character for all WarriorRPG characters.
 * Provides shared configuration and systems that both player-controlled
 * and AI-controlled characters need (health, combat, animation state).
 *
 * Implements IAbilitySystemInterface so the Gameplay Ability System can
 * discover the ASC on any character through a unified interface.
 *
 * Tick is disabled by default for performance — enable only in subclasses that need it.
 */
UCLASS()
class WARRIORRPG_API AWarriorBaseCharacter : public ACharacter, public IAbilitySystemInterface,
                                             public IPawnCombatInterface, public IPawnUIInterface
{
	GENERATED_BODY()

public:
	AWarriorBaseCharacter();

	/** Returns the project-specific ASC. Use this when you need access to custom ASC functionality. */
	FORCEINLINE UWarriorAbilitySystemComponent* GetWarriorAbilitySystemComponent() const
	{
		return WarriorAbilitySystemComponent;
	}

	/** Returns the project-specific Attribute Set for direct attribute queries. */
	FORCEINLINE UWarriorAttributeSet* GetWarriorAttributeSet() const
	{
		return WarriorAttributeSet;
	}

	//~ Begin IAbilitySystemInterface Interface
	/** Returns the ASC as the base engine type. Required by GAS to discover the component. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface Interface

	//~ Begin IPawnCombatInterface Interface
	/** Returns nullptr by default — subclasses override to return their combat component. */
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	//~ End IPawnCombatInterface Interface

	//~ Begin IPawnUIInterface Interface
	/**
	 * Returns nullptr by default — subclasses override to return their UI component.
	 * The base implementation satisfies the interface contract for characters that
	 * have not yet set up a UI component (e.g., NPCs or stub characters).
	 */
	virtual UPawnUIComponent* GetPawnUIComponent() const override;

	/**
	 * Returns nullptr by default — only AWarriorHeroCharacter overrides this.
	 * Satisfies the IPawnUIInterface contract on the base class so enemies and
	 * NPCs inherit a safe null return without boilerplate overrides.
	 */
	virtual UHeroUIComponent* GetHeroUIComponent() const override;

	/**
	 * Returns nullptr by default — only AWarriorEnemyCharacter overrides this.
	 * Satisfies the IPawnUIInterface contract on the base class so heroes and
	 * NPCs inherit a safe null return without boilerplate overrides.
	 */
	virtual UEnemyUIComponent* GetEnemyUIComponent() const override;
	//~ End IPawnUIInterface Interface

protected:
	//~ Begin APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface

	/** The Ability System Component that manages abilities, effects, and attributes for this character. */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly,
		Category = "AbilitySystem")
	TObjectPtr<UWarriorAbilitySystemComponent> WarriorAbilitySystemComponent;

	/** The Attribute Set that holds gameplay attributes (health, stamina, etc.) for this character. */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly,
		Category = "AbilitySystem")
	TObjectPtr<UWarriorAttributeSet> WarriorAttributeSet;

	/**
	 * Motion Warping component shared by all character types.
	 * Allows animation root motion to be redirected toward dynamic targets at runtime —
	 * used for attack lunges, ledge grabs, and any ability that needs precise
	 * positional alignment without hand-authored animation variants.
	 * Placed on the base character so both hero and enemy abilities can use it uniformly.
	 */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly,
		Category = "MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	/**
	 * Startup data asset that defines this character's initial abilities.
	 * Assigned in the Blueprint defaults — the hero and each enemy type
	 * will have their own concrete startup data asset.
	 *
	 * Declared as TSoftObjectPtr so the asset is NOT loaded into memory
	 * at engine startup or when the character class is loaded. It's only
	 * loaded on demand during PossessedBy via LoadSynchronous(), which keeps
	 * the initial memory footprint small and allows for faster level streaming.
	 */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "AbilitySystem")
	TSoftObjectPtr<UDataAsset_StartupDataBase> CharacterStartupData;
};
