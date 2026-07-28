// WarriorAttributeSet.h

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "WarriorAttributeSet.generated.h"

class IPawnUIInterface;

/**
 * Convenience macro that generates the four standard accessors for a GAS attribute:
 * - Static FGameplayAttribute getter    (used by GAS internally for attribute lookup)
 * - float value getter                  (Get<PropertyName>())
 * - float value setter                  (Set<PropertyName>())
 * - float value initializer             (Init<PropertyName>()) — used in the constructor
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)           \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)               \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)               \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UWarriorAttributeSet
 *
 * Base attribute set for all WarriorRPG characters (hero and enemies).
 * Attributes are registered automatically with the owning ASC at construction.
 *
 * All FGameplayAttributeData members are private — external code must use
 * the generated accessors (e.g. GetCurrentHealth(), SetCurrentHealth()).
 * Direct member access bypasses GAS replication and prediction.
 *
 * Initial values are set to 1.0f in the constructor. Real values are applied
 * by a GE_Initialize Gameplay Effect when the character is possessed.
 *
 * DamageTaken is an intermediary attribute — it is written by UGEExecCalc_DamageTaken
 * and consumed immediately in PostGameplayEffectExecute. It should never hold
 * a persistent non-zero value between GE applications.
 */
UCLASS()
class WARRIORRPG_API UWarriorAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

    // Grant ExecCalc structs direct access to attribute members for capture definition.
    // This is the standard GAS pattern — keeps attributes private everywhere else.
    friend struct FWarriorDamageCapture;

public:
    UWarriorAttributeSet();

    /**
     * Called by GAS after any modifier has been applied to an attribute.
     *
     * Handles three responsibilities:
     * 1. Clamping — keeps CurrentHealth and CurrentRage within [0, Max] after any direct modification.
     * 2. UI notification — broadcasts normalized attribute values when the avatar provides a UI component.
     * 3. Damage consumption — reads DamageTaken (written by UGEExecCalc_DamageTaken),
     *    applies it to CurrentHealth, then resets DamageTaken to zero.
     *
     * @param Data  Contains the attribute that was just modified and the source GE spec.
     */
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

private:
    UPROPERTY(BlueprintReadOnly,
        Category = "Health",
        meta = (AllowPrivateAccess = "true"))
    FGameplayAttributeData CurrentHealth;
    ATTRIBUTE_ACCESSORS(UWarriorAttributeSet,
                        CurrentHealth)

    UPROPERTY(BlueprintReadOnly,
        Category = "Health",
        meta = (AllowPrivateAccess = "true"))
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UWarriorAttributeSet,
                        MaxHealth)

    UPROPERTY(BlueprintReadOnly,
        Category = "Rage",
        meta = (AllowPrivateAccess = "true"))
    FGameplayAttributeData CurrentRage;
    ATTRIBUTE_ACCESSORS(UWarriorAttributeSet,
                        CurrentRage)

    UPROPERTY(BlueprintReadOnly,
        Category = "Rage",
        meta = (AllowPrivateAccess = "true"))
    FGameplayAttributeData MaxRage;
    ATTRIBUTE_ACCESSORS(UWarriorAttributeSet,
                        MaxRage)

    UPROPERTY(BlueprintReadOnly,
        Category = "Damage",
        meta = (AllowPrivateAccess = "true"))
    FGameplayAttributeData AttackPower;
    ATTRIBUTE_ACCESSORS(UWarriorAttributeSet,
                        AttackPower)

    UPROPERTY(BlueprintReadOnly,
        Category = "Damage",
        meta = (AllowPrivateAccess = "true"))
    FGameplayAttributeData DefensePower;
    ATTRIBUTE_ACCESSORS(UWarriorAttributeSet,
                        DefensePower)

    UPROPERTY(BlueprintReadOnly,
        Category = "Damage",
        meta = (AllowPrivateAccess = "true"))
    FGameplayAttributeData DamageTaken;
    ATTRIBUTE_ACCESSORS(UWarriorAttributeSet,
                        DamageTaken)

    /**
     * Cached weak reference to the avatar actor's optional IPawnUIInterface.
     * Populated on first access in PostGameplayEffectExecute and reused on subsequent calls
     * to avoid repeated interface casts per GE execution.
     * TWeakInterfacePtr automatically invalidates if the actor is destroyed.
     */
    TWeakInterfacePtr<IPawnUIInterface> CachedPawnUIInterface;
};
