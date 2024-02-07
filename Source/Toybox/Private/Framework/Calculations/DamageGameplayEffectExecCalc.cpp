// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Calculations/DamageGameplayEffectExecCalc.h"
#include "Framework/AttributeSets/HealthAttributeSet.h"
#include "Framework/AttributeSets/UtilityAttributeSet.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


struct FToyboxDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageHealth);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageShield);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Shield);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AbsoluteDamageBuff);

	FToyboxDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UHealthAttributeSet, DamageHealth, Target, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UHealthAttributeSet, DamageShield, Target, true);
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UHealthAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UHealthAttributeSet, Shield, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UUtilityAttributeSet, AbsoluteDamageBuff, Source, true);
	}
};

static const FToyboxDamageStatics& DamageStatics()
{
	static FToyboxDamageStatics DStatics;
	return DStatics;
}


UDamageGameplayEffectExecCalc::UDamageGameplayEffectExecCalc()
{
	RelevantAttributesToCapture.Add(DamageStatics().DamageHealthDef);
	RelevantAttributesToCapture.Add(DamageStatics().DamageShieldDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ShieldDef);
	RelevantAttributesToCapture.Add(DamageStatics().AbsoluteDamageBuffDef);
}

void UDamageGameplayEffectExecCalc::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UDamageGameplayEffectExecCalc::Execute_Implementation)

	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();
	TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// Get toybox tags
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// First get the values that will effect how much damage we will do

	float Armor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef,
		EvaluationParameters, Armor);
	
	Armor = FMath::Max<float>(Armor, 0.0f);

	// We will also need the current shield value
	float ShieldValue = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ShieldDef,
		EvaluationParameters, ShieldValue);
	
	// Work out total damage then split between health and shield damage
	
	float TotalDamage = 0.0f;
	// Capture optional damage value set on the damage GE as a CalculationModifier under the ExecutionCalculation
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageHealthDef,
		EvaluationParameters, TotalDamage);

	float AbsoluteDamageBuff = 0.0f;
	// Capture optional damage buff that might be applied to the source
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AbsoluteDamageBuffDef,
		EvaluationParameters, AbsoluteDamageBuff);

	TotalDamage += AbsoluteDamageBuff;
	
	TotalDamage += FMath::Max<float>(Spec.GetSetByCallerMagnitude(
		NativeTags.MetaTag_Damage, false, -1.0f), 0.0f);

	if (TotalDamage < UE_FLOAT_NORMAL_THRESH)
	{
		return;
	}
	
	// Now add the modifiers
	const float DamageAfterShield = FMath::Max<float>(TotalDamage - ShieldValue, 0.0f);

	float ShieldDamage = 0.0f;
	float HealthDamage = FMath::Max<float>(DamageAfterShield, 0.0f);
	if (DamageAfterShield > UE_FLOAT_NORMAL_THRESH)
	{
		ShieldDamage = ShieldValue;

		HealthDamage -= HealthDamage * Armor;
	}
	else
	{
		ShieldDamage = TotalDamage;
	}
	
	if (ShieldDamage > UE_FLOAT_NORMAL_THRESH)
	{
		// Set the Target's damage meta attribute
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageShieldProperty,
			EGameplayModOp::Additive, ShieldDamage));
	}
	
	if (HealthDamage > UE_FLOAT_NORMAL_THRESH)
	{
		// Set the Target's damage meta attribute
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageHealthProperty,
			EGameplayModOp::Additive, HealthDamage));
	}
}
