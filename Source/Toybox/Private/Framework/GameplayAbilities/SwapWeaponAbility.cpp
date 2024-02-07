// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/SwapWeaponAbility.h"

#include "AbilitySystemComponent.h"
#include "HelperObjects/ToyboxGameplayTags.h"

void USwapWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) || ActorInfo == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = ActorInfo->AbilitySystemComponent;
	if (!AbilitySystemComponent.IsValid() || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	FGameplayTagContainer AbilitiesToActivate;
	const FToyboxGameplayTags& ToyboxGameplayTags = FToyboxGameplayTags::Get();
	if (AbilitySystemComponent->HasMatchingGameplayTag(ToyboxGameplayTags.Weapon_Ranged))
	{
		AbilitiesToActivate.AddTag(ToyboxGameplayTags.AbilityTag_Combat_DrawMelee);
	}
	else if (AbilitySystemComponent->HasMatchingGameplayTag(ToyboxGameplayTags.Weapon_Melee))
	{
		AbilitiesToActivate.AddTag(ToyboxGameplayTags.AbilityTag_Combat_DrawRanged);
	}
	else if (AbilitySystemComponent->HasMatchingGameplayTag(ToyboxGameplayTags.Weapon_Unarmed))
	{
		AbilitiesToActivate.AddTag(ToyboxGameplayTags.AbilityTag_Combat_DrawRanged);
	}
	
	AbilitySystemComponent->TryActivateAbilitiesByTag(AbilitiesToActivate);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
