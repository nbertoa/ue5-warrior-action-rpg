// WarriorEnemyCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "Characters/WarriorBaseCharacter.h"
#include "WarriorEnemyCharacter.generated.h"

class UBoxComponent;
class UEnemyCombatComponent;
class UEnemyUIComponent;
class UWidgetComponent;

/**
 * AWarriorEnemyCharacter
 *
 * Base class for all AI-controlled enemy characters.
 * Extends AWarriorBaseCharacter with enemy-specific combat and UI components,
 * AI possession configuration, and body collision boxes for unarmed melee attacks.
 *
 * Mirrors AWarriorHeroCharacter in structure — both share the same base class
 * but specialize for their respective factions. Enemy characters are automatically
 * possessed by AWarriorAIController when placed in the world or spawned at runtime.
 *
 * Startup abilities and attributes are loaded asynchronously from CharacterStartupData
 * in PossessedBy, so the enemy is fully functional as soon as the AI controller takes
 * control regardless of when the asset finishes loading.
 */
UCLASS()
class WARRIORRPG_API AWarriorEnemyCharacter : public AWarriorBaseCharacter
{
	GENERATED_BODY()

public:
	AWarriorEnemyCharacter();

	/**
	 * Returns the enemy-specific combat component.
	 * Used by abilities and AI controllers to query and drive combat state
	 * without casting to a concrete enemy character subclass.
	 */
	FORCEINLINE UEnemyCombatComponent* GetEnemyCombatComponent() const
	{
		return EnemyCombatComponent;
	}

	/**
	 * Returns the left hand body collision box.
	 * Exposed so UEnemyCombatComponent can toggle collision during attack animations
	 * without holding a direct reference to the character.
	 */
	FORCEINLINE UBoxComponent* GetLeftHandCollisionBox() const
	{
		return LeftHandCollisionBox;
	}

	/**
	 * Returns the right hand body collision box.
	 * Exposed so UEnemyCombatComponent can toggle collision during attack animations
	 * without holding a direct reference to the character.
	 */
	FORCEINLINE UBoxComponent* GetRightHandCollisionBox() const
	{
		return RightHandCollisionBox;
	}

	//~ Begin IPawnCombatInterface Interface
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	//~ End IPawnCombatInterface Interface

	//~ Begin IPawnUIInterface Interface
	/**
	 * Returns the enemy's UI component cast to the base UPawnUIComponent type.
	 * Used by the attribute set to broadcast health changes to the enemy's
	 * overhead health bar without knowing the concrete component type.
	 */
	virtual UPawnUIComponent* GetPawnUIComponent() const override;

	/**
	 * Returns the enemy's UI component as its concrete UEnemyUIComponent type.
	 * Used by external systems that need access to enemy-exclusive UI functionality
	 * (e.g., RegisterEnemyDrawnWidget, RemoveEnemyDrawnWidgetsIfAny) without
	 * casting to a specific enemy character subclass.
	 *
	 * @return The enemy's UEnemyUIComponent. Always valid on a live enemy instance.
	 */
	virtual UEnemyUIComponent* GetEnemyUIComponent() const override;
	//~ End IPawnUIInterface Interface

protected:
	//~ Begin AActor Interface
	/**
	 * Retrieves the health widget from EnemyHealthWidgetComponent and initializes
	 * it as an enemy-owned widget via InitEnemyCreatedWidget, binding it to
	 * this character's EnemyUIComponent delegates.
	 */
	virtual void BeginPlay() override;
	//~ End AActor Interface

	//~ Begin APawn Interface
	/**
	 * Triggers asynchronous loading of CharacterStartupData and grants all startup
	 * abilities and attributes to the enemy's ASC once loaded.
	 * Called automatically when AWarriorAIController takes possession.
	 */
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	/**
	 * Re-attaches LeftHandCollisionBox or RightHandCollisionBox to the updated bone
	 * when their attach bone name properties are changed in the Details panel.
	 * Only compiled in editor builds — no runtime cost in packaged games.
	 */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif

	/** Enemy-specific combat component. Handles block evaluation, unblockable bypass,
	 *  and body collision toggling for AI-driven attack sequences. */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Combat")
	TObjectPtr<UEnemyCombatComponent> EnemyCombatComponent;

	/** Left hand body collision box for unarmed melee attacks.
	 *  Attached to the mesh bone specified by LeftHandCollisionBoxAttachBoneName.
	 *  Collision is toggled by UEnemyCombatComponent via Animation Notifies. */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Combat")
	TObjectPtr<UBoxComponent> LeftHandCollisionBox;

	/** Bone name on the skeletal mesh to which LeftHandCollisionBox is attached.
	 *  Assign in Blueprint defaults. Changing this in the Details panel re-attaches
	 *  the box at editor time via PostEditChangeProperty. */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat")
	FName LeftHandCollisionBoxAttachBoneName;

	/** Right hand body collision box for unarmed melee attacks.
	 *  Attached to the mesh bone specified by RightHandCollisionBoxAttachBoneName.
	 *  Collision is toggled by UEnemyCombatComponent via Animation Notifies. */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Combat")
	TObjectPtr<UBoxComponent> RightHandCollisionBox;

	/** Bone name on the skeletal mesh to which RightHandCollisionBox is attached.
	 *  Assign in Blueprint defaults. Changing this in the Details panel re-attaches
	 *  the box at editor time via PostEditChangeProperty. */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat")
	FName RightHandCollisionBoxAttachBoneName;

	/**
	 * Enemy-specific UI component.
	 * Owns the delegates that drive enemy UI elements such as overhead health bars.
	 * The attribute set broadcasts to OnCurrentHealthChanged after modifying health.
	 * Also tracks all drawn widgets for cleanup on death via RemoveEnemyDrawnWidgetsIfAny.
	 */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly,
		Category = "UI")
	TObjectPtr<UEnemyUIComponent> EnemyUIComponent;

	/**
	 * Widget component that renders the enemy's overhead health bar in world space.
	 * Attached to the mesh so it follows the character's position and rotation.
	 * The widget class is assigned in Blueprint defaults.
	 * Initialized in BeginPlay via InitEnemyCreatedWidget to bind UI delegates.
	 */
	UPROPERTY(VisibleAnywhere,
		BlueprintReadOnly,
		Category = "UI")
	TObjectPtr<UWidgetComponent> EnemyHealthWidgetComponent;

	/**
	 * Overlap callback shared by both hand collision boxes.
	 * Casts OtherActor to APawn and forwards to EnemyCombatComponent::OnHitTargetActor
	 * if the target is hostile. Registered via AddUniqueDynamic in the constructor
	 * so both boxes share a single implementation.
	 */
	UFUNCTION()
	virtual void OnBodyCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	                                            AActor* OtherActor,
	                                            UPrimitiveComponent* OtherComp,
	                                            int32 OtherBodyIndex,
	                                            bool bFromSweep,
	                                            const FHitResult& SweepResult);

private:
	/** Asynchronously loads CharacterStartupData and grants all startup
	 *  abilities and attributes to the enemy's ASC. Called from PossessedBy. */
	void InitEnemyStartupData();

	/** Callback invoked by the Streamable Manager when CharacterStartupData
	 *  has finished loading. Applies the loaded data to WarriorAbilitySystemComponent. */
	void OnStartupDataLoaded();
};
