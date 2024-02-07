// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/ForceShield.h"

#include "HelperObjects/ToyboxGameplayTags.h"

UForceShield::UForceShield()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	MetaShieldTag = NativeTags.MetaTag_Shield;
}

void UForceShield::ExecuteAttacker(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ExecuteAttacker(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	TArray<TWeakObjectPtr<UAbilitySystemComponent>> AbilitySystemComponents;
	GetAllAbilitySystemsOfTeamInRadius(AttackerTag, AttackerRadius, true, AbilitySystemComponents);

	const FGameplayEffectSpecHandle SpecHandle = GetEffectHandle(AttackerEffect);
		
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	ApplySetByCallerMagnitude(MetaShieldTag, AttackerShield, SpecHandle.Data.Get());

	ApplyEffectToTargets(SpecHandle.Data.Get(), AbilitySystemComponents);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UForceShield::ExecuteDefender(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ExecuteDefender(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const FGameplayEffectSpecHandle SpecHandle = GetEffectHandle(DefenderEffect);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	ApplySetByCallerMagnitude(MetaShieldTag, DefenderShield, SpecHandle.Data.Get());

	ApplyEffectToTarget(SpecHandle.Data.Get(), ActorInfo->AbilitySystemComponent);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
