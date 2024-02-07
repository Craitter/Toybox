// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/ScalesOfPower.h"

#include "HelperObjects/ToyboxGameplayTags.h"

UScalesOfPower::UScalesOfPower()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	MetaDurationTag = NativeTags.MetaTag_Duration;
	MetaAbsoluteDamageBuffTag = NativeTags.MetaTag_Buff_DamageAbsolute;
	
}

void UScalesOfPower::ExecuteAttacker(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
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

	ApplySetByCallerMagnitude(MetaDurationTag, AttackerDuration, SpecHandle.Data.Get());
	ApplySetByCallerMagnitude(MetaAbsoluteDamageBuffTag, AttackerAbsoluteDamageBuff, SpecHandle.Data.Get());
	
	ApplyEffectToTargets(SpecHandle.Data.Get(), AbilitySystemComponents);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UScalesOfPower::ExecuteDefender(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ExecuteDefender(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	TArray<TWeakObjectPtr<UAbilitySystemComponent>> AbilitySystemComponents;
	GetAllAbilitySystemsOfTeamInRadius(AttackerTag, AttackerRadius, true, AbilitySystemComponents);

	const FGameplayEffectSpecHandle SpecHandle = GetEffectHandle(DefenderEffect);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	ApplySetByCallerMagnitude(MetaDurationTag, DefenderDuration, SpecHandle.Data.Get());
	ApplySetByCallerMagnitude(MetaAbsoluteDamageBuffTag, DefenderAbsoluteDamageBuffPerAttacker * AbilitySystemComponents.Num(), SpecHandle.Data.Get());

	ApplyEffectToTarget(SpecHandle.Data.Get(), ActorInfo->AbilitySystemComponent);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
