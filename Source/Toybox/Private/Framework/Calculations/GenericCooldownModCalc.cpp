// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Calculations/GenericCooldownModCalc.h"

#include "Framework/ToyboxGameplayAbility.h"

float UGenericCooldownModCalc::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const UToyboxGameplayAbility* Ability = Cast<UToyboxGameplayAbility>(Spec.GetContext().GetAbilityInstance_NotReplicated());

	if (!IsValid(Ability))
	{
		return 0.0f;
	}
	return Ability->GetCooldownDuration().GetValueAtLevel(Ability->GetAbilityLevel());
}
