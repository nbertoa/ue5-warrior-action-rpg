#include "GEExecCalc/GEExecCalc_DamageTaken.h"

#include "AbilitySystem/WarriorAttributeSet.h"
#include "Utils/WarriorRPGTags.h"

struct FWarriorDamageCapture
{
    // DECLARE_ATTRIBUTE_CAPTUREDEF generates two members per attribute:
    // - <AttributeName>Property  (FProperty* used for reflection)
    // - <AttributeName>Def       (FGameplayEffectAttributeCaptureDefinition used by GAS)
    // Both are needed by RelevantAttributesToCapture and ExecuteImplementation.
    DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
    DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower)
    DECLARE_ATTRIBUTE_CAPTUREDEF(DamageTaken)

    FWarriorDamageCapture()
    {
        // DEFINE_ATTRIBUTE_CAPTUREDEF(AttributeSetClass, AttributeName, SourceOrTarget, bSnapshot)
        // Source = capture from whoever applied the GE (the attacker).
        // bSnapshot = false means the value is read at execution time, not at spec creation time.
        // This ensures AttackPower reflects any buffs applied between spec creation and execution.
        DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet,
                                    AttackPower,
                                    Source,
                                    false) // bSnapshot

        // Target = capture from whoever receives the GE (the character being damaged).
        // DefensePower is read from the target at execution time for the same reason.
        DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet,
                                    DefensePower,
                                    Target,
                                    false) // bSnapshot

        DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet,
                                    DamageTaken,
                                    Target,
                                    false) // bSnapshot
    }
};

// Static singleton pattern: the capture struct is constructed once and reused.
// Constructing it every time Execute is called would be wasteful — the definitions
// never change between executions.
static const FWarriorDamageCapture& GetWarriorDamageCapture()
{
    static FWarriorDamageCapture WarriorDamageCapture;
    return WarriorDamageCapture;
}

UGEExecCalc_DamageTaken::UGEExecCalc_DamageTaken()
{
    // Register which attributes this calculation needs GAS to capture before Execute runs.
    // Without registering them here, the captured values will be zero at execution time.
    RelevantAttributesToCapture.Add(GetWarriorDamageCapture().AttackPowerDef);
    RelevantAttributesToCapture.Add(GetWarriorDamageCapture().DefensePowerDef);
    RelevantAttributesToCapture.Add(GetWarriorDamageCapture().DamageTakenDef);
}

void UGEExecCalc_DamageTaken::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& InExecutionParams,
                                                     FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& EffectSpec = InExecutionParams.GetOwningSpec();

    // Build the evaluator context with source and target tags.
    // These tags are used by GAS to filter out captured attribute values
    // that shouldn't apply under certain tag conditions (e.g., immunity tags).
    FAggregatorEvaluateParameters EvaluateParameters;
    EvaluateParameters.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
    EvaluateParameters.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

    // Read AttackPower from the source's ASC.
    // Returns false if the attribute wasn't captured — value stays 0.0f in that case.
    float SourceAttackPower = 0.0f;
    InExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetWarriorDamageCapture().AttackPowerDef,
                                                                 EvaluateParameters,
                                                                 SourceAttackPower);
    // Extract SetByCaller magnitudes injected by MakeHeroDamageEffectSpecHandle.
    // Iterating the map instead of calling GetSetByCallerMagnitude individually
    // avoids multiple map lookups and handles missing tags gracefully (value stays 0).
    float BaseDamage = 0.0f;
    int32 UsedLightAttackComboCount = 0;
    int32 UsedHeavyAttackComboCount = 0;

    for (const TPair<FGameplayTag, float>& TagMagnitude : EffectSpec.SetByCallerTagMagnitudes)
    {
        if (TagMagnitude.Key.MatchesTagExact(WarriorRPGTags::Shared::SetByCaller::BaseDamage))
        {
            BaseDamage = TagMagnitude.Value;
        }

        if (TagMagnitude.Key.MatchesTagExact(WarriorRPGTags::Player::SetByCaller::AttackType::Light))
        {
            // static_cast<int32>: TagMagnitude.Value is float — the cast is explicit and intentional.
            // Combo counts are always whole numbers; the float representation is an artifact
            // of SetByCaller storing all magnitudes as floats.
            UsedLightAttackComboCount = static_cast<int32>(TagMagnitude.Value);
        }

        if (TagMagnitude.Key.MatchesTagExact(WarriorRPGTags::Player::SetByCaller::AttackType::Heavy))
        {
            // Same rationale as UsedLightAttackComboCount above.
            UsedHeavyAttackComboCount = static_cast<int32>(TagMagnitude.Value);
        }
    }

    // Read DefensePower from the target's ASC.
    // Captured separately from AttackPower because it belongs to the target, not the source.
    float TargetDefensePower = 0.0f;
    InExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetWarriorDamageCapture().DefensePowerDef,
                                                                 EvaluateParameters,
                                                                 TargetDefensePower);

    if (UsedLightAttackComboCount != 0)
    {
        const float DamageIncreasePercentLight = (UsedLightAttackComboCount - 1) * 0.05f + 1.0f;

        BaseDamage *= DamageIncreasePercentLight;
    }

    if (UsedHeavyAttackComboCount != 0)
    {
        const float DamageIncreasePercentHeavy = UsedHeavyAttackComboCount * 0.15f + 1.0f;

        BaseDamage *= DamageIncreasePercentHeavy;
    }

    // Defense is initialized to 1, but a malformed or incomplete initialization
    // effect can still leave it at zero. Clamp the divisor to avoid Inf/NaN damage.
    const float SafeTargetDefensePower = FMath::Max(TargetDefensePower, 1.0f);
    const float FinalDamageDone = BaseDamage * SourceAttackPower / SafeTargetDefensePower;
    if (FinalDamageDone > 0.0f)
    {
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(GetWarriorDamageCapture().DamageTakenProperty,
                                                                            EGameplayModOp::Override,
                                                                            FinalDamageDone));
    }
}
