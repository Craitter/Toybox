// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/MilitaryZeal.h"

#include "HelperObjects/ToyboxGameplayTags.h"

UMilitaryZeal::UMilitaryZeal()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	MetaHealTag = NativeTags.MetaTag_Heal;
}

void UMilitaryZeal::ExecuteAttacker(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

	ApplySetByCallerMagnitude(MetaHealTag, AttackerHeal, SpecHandle.Data.Get());
	
	ApplyEffectToTargets(SpecHandle.Data.Get(), AbilitySystemComponents);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UMilitaryZeal::ExecuteDefender(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ExecuteDefender(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	TArray<TWeakObjectPtr<UAbilitySystemComponent>> AbilitySystemComponents;
	GetAllAbilitySystemsOfTeamInRadius(AttackerTag, DefenderRadius, true, AbilitySystemComponents);

	const FGameplayEffectSpecHandle SpecHandle = GetEffectHandle(DefenderEffect);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	ApplySetByCallerMagnitude(MetaHealTag, DefenderHealPerAttacker * AbilitySystemComponents.Num(), SpecHandle.Data.Get());

	ApplyEffectToTarget(SpecHandle.Data.Get(), ActorInfo->AbilitySystemComponent);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
