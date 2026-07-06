// WarriorFunctionLibrary.h
// Global Blueprint Function Library for WarriorRPG.
// Centralizes GAS utility operations used across multiple abilities and Blueprints:
// ASC lookup, loose gameplay tag management, tag presence queries, and UI widget lookup.
//
// Design: each operation has a Native (C++) variant and optionally a BP_ variant.
// The Native variants are faster (no Blueprint overhead) and used from C++ abilities.
// The BP_ variants add Blueprint-friendly output formats (e.g., ExpandEnumAsExecs).

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WarriorTypes/WarriorEnumTypes.h"
#include "WarriorFunctionLibrary.generated.h"

class UPawnCombatComponent;
class UWarriorAbilitySystemComponent;
class UWarriorActivatableWidget;
struct FScalableFloat;

/**
 * Static utility library for WarriorRPG.
 * All functions are stateless — no instance is needed.
 * Native functions are C++-only; BlueprintCallable functions are accessible from both.
 */
UCLASS()
class WARRIORRPG_API UWarriorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Retrieves the UWarriorAbilitySystemComponent from any actor that implements
	 * IAbilitySystemInterface. Used internally by all other functions in this library.
	 *
	 * @param InActor   The actor to query. Must not be null and must have an ASC.
	 * @return          The actor's ASC cast to UWarriorAbilitySystemComponent,
	 *                  or nullptr if the actor has no ASC (logs an ensure and returns null).
	 */
	static UWarriorAbilitySystemComponent* NativeGetWarriorASCFromActor(AActor* InActor);

	/**
	 * Adds InTag as a loose gameplay tag to InActor's ASC if not already present.
	 * Loose tags are not replicated and not tied to any ability or effect —
	 * use them for transient state flags (e.g., "is equipping", "is staggered").
	 *
	 * @param InActor   The actor whose ASC receives the tag. Must have an ASC.
	 * @param InTag     The tag to add. No-op if the actor already has it.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|FunctionLibrary")
	static void AddGameplayTagToActor(AActor* InActor,
	                                  FGameplayTag InTag);

	/**
	 * Removes InTag from InActor's ASC if present.
	 * Safe to call even if the actor does not have the tag — no-op in that case.
	 *
	 * @param InActor   The actor whose ASC loses the tag. Must have an ASC.
	 * @param InTag     The tag to remove.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|FunctionLibrary")
	static void RemoveGameplayTagFromActor(AActor* InActor,
	                                       FGameplayTag InTag);

	/**
	 * Returns true if InActor's ASC has InTag (via HasMatchingGameplayTag).
	 * Use from C++ — for Blueprints, prefer BP_DoesActorHaveTag.
	 *
	 * @param InActor   The actor to query. Must have an ASC.
	 * @param InTag     The tag to check for.
	 * @return          True if the actor's ASC has a matching tag.
	 */
	static bool NativeDoesActorHaveTag(AActor* InActor,
	                                   FGameplayTag InTag);

	/**
	 * Blueprint-friendly tag presence check with branch-style execution pins.
	 * ExpandEnumAsExecs converts OutConfirmType into Yes/No execution pins on the
	 * Blueprint node, so no Branch node is needed after calling this.
	 *
	 * @param InActor           The actor to query. Must have an ASC.
	 * @param InTag             The tag to check for.
	 * @param OutConfirmType    Set to Yes if the actor has the tag, No otherwise.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|FunctionLibrary",
		meta = (DisplayName = "Does Actor Have Tag", ExpandEnumAsExecs = "OutConfirmType"))
	static void BP_DoesActorHaveTag(AActor* InActor,
	                                FGameplayTag InTag,
	                                EWarriorConfirmType& OutConfirmType);

	/**
	 * Retrieves the UPawnCombatComponent from any actor that implements
	 * IPawnCombatInterface. Used internally by BP_GetPawnCombatComponentFromActor.
	 *
	 * @param InActor   The actor to query. Must not be null.
	 * @return          The actor's combat component, or nullptr if the actor
	 *                  does not implement IPawnCombatInterface.
	 */
	static UPawnCombatComponent* NativeGetPawnCombatComponentFromActor(AActor* InActor);

	/**
	 * Blueprint-friendly combat component lookup with branch-style execution pins.
	 * ExpandEnumAsExecs converts OutValidType into Valid/Invalid execution pins,
	 * so no Branch node is needed after calling this in Blueprint.
	 *
	 * @param InActor       The actor to query. Must not be null.
	 * @param OutValidType  Set to Valid if a combat component was found, Invalid otherwise.
	 * @return              The actor's combat component, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|FunctionLibrary",
		meta = (DisplayName = "Get Pawn Combat Component From Actor", ExpandEnumAsExecs = "OutValidType"))
	static UPawnCombatComponent* BP_GetPawnCombatComponentFromActor(AActor* InActor,
	                                                                EWarriorValidType& OutValidType);

	// ─── UI ─────────────────────────────────────────────────────────────────

	/**
	 * Returns the widget Blueprint class mapped to the given Gameplay Tag
	 * in UWarriorUISettings::WidgetClassByTag.
	 * Crashes in development if no entry exists for the tag — this indicates
	 * the designer forgot to register the widget class in Project Settings.
	 *
	 * @param InWidgetTag   The tag identifying the desired widget class.
	 *                      Must belong to the WarriorRPGTags.UI.Widget hierarchy.
	 * @return              The widget class associated with the tag.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|UI")
	static TSubclassOf<UWarriorActivatableWidget> GetWidgetClassByTag(
		UPARAM(meta = (Categories = "WarriorRPGTags.UI.Widget")) FGameplayTag InWidgetTag);

	// ─── AI ─────────────────────────────────────────────────────────────────

	/**
	 * Returns true if TargetPawn belongs to a different team than QueryPawn.
	 * Resolves team affiliation by casting each pawn's controller to
	 * IGenericTeamAgentInterface and comparing their team IDs.
	 * Used by weapon overlap callbacks to filter hits to hostile targets only,
	 * preventing friendly fire between allies with the same team ID.
	 *
	 * Crashes in development if either pawn has no controller or the controller
	 * does not implement IGenericTeamAgentInterface — both are setup errors.
	 *
	 * @param QueryPawn     The pawn performing the hostility check. Must not be null.
	 * @param TargetPawn    The pawn being evaluated. Must not be null.
	 * @return              True if the pawns are on different teams.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|FunctionLibrary")
	static bool IsTargetPawnHostile(APawn* QueryPawn,
	                                APawn* TargetPawn);

	// ─── Utility ─────────────────────────────────────────────────────────────

	/**
	 * Evaluates a ScalableFloat curve at the given level and returns the result.
	 * Wraps FScalableFloat::GetValueAtLevel for Blueprint exposure — C++ callers
	 * should call GetValueAtLevel directly to avoid the function call overhead.
	 * CompactNodeTitle keeps the Blueprint node label concise in the graph.
	 *
	 * @param InScalableFloat   The curve to evaluate. Passed by const ref — not modified.
	 * @param InLevel           The level at which to evaluate the curve. Defaults to 1.0.
	 * @return                  The float value of the curve at InLevel.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|FunctionLibrary",
		meta = (CompactNodeTitle = "Get Value At Level"))
	static float GetScalableFloatValueAtLevel(const FScalableFloat& InScalableFloat,
	                                          float InLevel = 1.0f);

	/**
	 * Computes the hit react direction tag based on the angle between the victim's
	 * forward vector and the direction toward the attacker.
	 *
	 * Uses a dot + cross product pair to produce a signed angle in [-180°, +180°]:
	 * - Dot product gives the unsigned angular magnitude (via DegAcos).
	 * - Cross product Z component provides the sign (negative = attacker to the left).
	 *
	 * The signed angle is then mapped to one of four directional tags:
	 * - [-45°,  +45°] → Front
	 * - (-135°, -45°) → Left
	 * - (+45°, +135°] → Right
	 * - < -135° or > 135° → Back
	 *
	 * @param InAttacker            The actor that initiated the hit. Must not be null.
	 * @param InVictim              The actor receiving the hit. Must not be null.
	 * @param OutAngleDifference    The signed angle (in degrees) from the victim's forward
	 *                              to the attacker's direction. Negative = attacker to the left.
	 * @return                      The Gameplay Tag identifying the hit direction quadrant.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|FunctionLibrary")
	static FGameplayTag ComputeHitReactDirectionTag(AActor* InAttacker,
	                                                AActor* InVictim,
	                                                float& OutAngleDifference);

	/**
	 * Returns true if the defender is facing the attacker within the valid block arc.
	 *
	 * Uses the dot product of both actors' forward vectors to determine facing alignment.
	 * A result below -0.1 means the two forward vectors point in roughly opposite directions —
	 * the defender faces the attacker — which is the geometric precondition for a valid block.
	 *
	 * Threshold -0.1 corresponds to an inter-forward angle of ~96°, giving the defender
	 * a generous ±48° window on either side of directly facing the attacker.
	 * This value is intentionally permissive for the initial implementation.
	 *
	 * @param InAttacker    The actor that initiated the hit. Must not be null.
	 * @param InDefender    The actor attempting to block the hit. Must not be null.
	 * @return              True if the defender's facing satisfies the block geometry check.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|FunctionLibrary")
	static bool IsValidBlock(AActor* InAttacker,
	                         AActor* InDefender);

	/**
	 * Applies InSpecHandle from InInstigator's ASC to InTargetActor's ASC.
	 *
	 * Resolves both actors' ASCs via NativeGetWarriorASCFromActor. Either actor lacking
	 * a valid ASC is treated as a recoverable configuration error (e.g. a Blueprint-side
	 * caller passing an actor that does not implement IAbilitySystemInterface) — logs an
	 * ensure and returns false rather than crashing, since this function is BlueprintCallable
	 * and reachable from designer-authored graphs.
	 *
	 * @param InInstigator   The actor whose ASC is the source of the effect. Must have an ASC.
	 * @param InTargetActor  The actor whose ASC receives the effect. Must have an ASC.
	 * @param InSpecHandle   The gameplay effect spec to apply. Must be valid.
	 * @return               True if the effect was successfully applied; false if either ASC
	 *                       could not be resolved or the effect application itself failed.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|FunctionLibrary")
	static bool ApplyGameplayEffectSpecHandleToTargetActor(AActor* InInstigator,
	                                                       AActor* InTargetActor,
	                                                       const FGameplayEffectSpecHandle& InSpecHandle);
};
