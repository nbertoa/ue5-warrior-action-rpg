#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GEExecCalc_DamageTaken.generated.h"

/**
 * UGEExecCalc_DamageTaken
 *
 * Execution Calculation that computes the final damage taken by a target.
 * Captures AttackPower from the source and DefensePower from the target,
 * then applies a damage formula and writes the result back to the target's
 * CurrentHealth attribute via a modifier.
 *
 * Assigned in the GE_Hero_DealDamage Gameplay Effect asset under
 * Executions → Calculation Class.
 */
UCLASS()
class WARRIORRPG_API UGEExecCalc_DamageTaken : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    /** Registers AttackPower and DefensePower as attributes to capture
     *  when this calculation is executed. */
    UGEExecCalc_DamageTaken();

    /**
 * Computes the final damage and writes it back to the target's CurrentHealth.
 * Reads AttackPower from the source, DefensePower from the target, and all
 * SetByCaller magnitudes (BaseDamage, combo counts) from the effect spec.
 *
 * @param InExecutionParams     Provides access to captured attributes and the effect spec.
 * @param OutExecutionOutput    Receives the computed attribute modifiers to apply.
 */
    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& InExecutionParams,
                                        FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
