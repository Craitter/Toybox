// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/DrawWeaponAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Toybox/Toybox.h"


UDrawWeaponAbility::UDrawWeaponAbility()
{
		
}

void UDrawWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UDrawWeaponAbility::ActivateAbility)
	
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) || ActorInfo == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = ActorInfo->AbilitySystemComponent;
	if (!AbilitySystemComponent.IsValid() || AbilitySystemComponent->HasMatchingGameplayTag(WeaponTag) || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	const TWeakObjectPtr<AToyboxCharacter> Character = GetToyboxCharacterFromActorInfo();
	if (!Character.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	//adding and removing replicated and loose anyway, to match the server,
	//replicated will overwrite the tag count and not change it, so if its the same
	//it will not call the matching delegate
	const FGameplayTag WeaponTagToRemove = Character->GetCurrentWeapon();
	if (WeaponTagToRemove != FGameplayTag::EmptyTag)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(WeaponTagToRemove);
		AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(WeaponTagToRemove);
	}
	AbilitySystemComponent->AddLooseGameplayTag(WeaponTag);
	AbilitySystemComponent->AddReplicatedLooseGameplayTag(WeaponTag);

	if (ActorInfo->IsNetAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Currently Equipped: %s"), *WeaponTag.ToString());
	}
	
	if (IsValid(DrawAnimMontage))
	{
		PlayMontageAndWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("DrawMontageTask"), DrawAnimMontage);
		
		if (!IsValid(PlayMontageAndWaitTask))
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		PlayMontageAndWaitTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageTaskEnds);
		PlayMontageAndWaitTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageTaskEnds);
		PlayMontageAndWaitTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageTaskEnds);
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
	//Play Montage
}

void UDrawWeaponAbility::OnMontageTaskEnds()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
