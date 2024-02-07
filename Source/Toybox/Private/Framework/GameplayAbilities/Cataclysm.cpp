// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/Cataclysm.h"

#include "HelperObjects/ToyboxGameplayTags.h"

UCataclysm::UCataclysm()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	MetaDamageTag = NativeTags.MetaTag_Damage;
}

void UCataclysm::ExecuteAttacker(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ExecuteAttacker(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	TArray<TWeakObjectPtr<UAbilitySystemComponent>> AbilitySystemComponents;
	GetAllAbilitySystemsOfTeamInRadius(DefenderTag,
		AttackerRadius,
		false,
		AbilitySystemComponents,
		!bIgnoreBlockingTerrain,
		RadiusThatAlwaysHits);

	const FGameplayEffectSpecHandle SpecHandle = GetEffectHandle(AttackerEffect);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	ApplySetByCallerMagnitude(MetaDamageTag, DamageAsAttacker, SpecHandle.Data.Get());
	
	ApplyEffectToTargets(SpecHandle.Data.Get(), AbilitySystemComponents);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UCataclysm::ExecuteDefender(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ExecuteDefender(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	TArray<TWeakObjectPtr<UAbilitySystemComponent>> AbilitySystemComponents;
	GetAllAbilitySystemsOfTeamInRadius(AttackerTag, DefenderRadius, true,
		AbilitySystemComponents, !bIgnoreBlockingTerrain, RadiusThatAlwaysHits);

	const FGameplayEffectSpecHandle SpecHandle = GetEffectHandle(DefenderEffect);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	ApplySetByCallerMagnitude(MetaDamageTag, DamageAsDefender, SpecHandle.Data.Get());

	ApplyEffectToTargets(SpecHandle.Data.Get(), AbilitySystemComponents);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
