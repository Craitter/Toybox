// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/ReloadAbility.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Actors/RangedWeapon.h"
#include "Toybox/Toybox.h"




UReloadAbility::UReloadAbility()
{
	bServerRespectsRemoteAbilityCancellation = false;
}

void UReloadAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UReloadAbility::ActivateAbility)

	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) || ActorInfo == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (!ensure(ReloadEffect != nullptr))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	const TWeakObjectPtr<AToyboxCharacter> Character = CastChecked<AToyboxCharacter>(ActorInfo->AvatarActor.Get());

	if (!Character.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	AbilitySystemComponent = Character->GetAbilitySystemComponent();

	Weapon = Character->GetEquippedRanged();

	if (!Weapon.IsValid() || !AbilitySystemComponent.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	const TWeakObjectPtr<UAnimMontage> ReloadAnimation = Weapon->GetReloadAnimation();

	if (!ReloadAnimation.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	PlayMontageAndWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("ReloadMontageTask"),
			ReloadAnimation.Get());

	if (!IsValid(PlayMontageAndWaitTask))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	PlayMontageAndWaitTask->OnCompleted.AddDynamic(this, &ThisClass::OnReloadTaskCompleted);
	
	PlayMontageAndWaitTask->OnCancelled.AddDynamic(this, &ThisClass::OnReloadTaskFailed);
	PlayMontageAndWaitTask->OnInterrupted.AddDynamic(this, &ThisClass::OnReloadTaskFailed);
	
	PlayMontageAndWaitTask->ReadyForActivation();
}

void UReloadAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UReloadAbility::OnReloadTaskFailed()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UReloadAbility::OnReloadTaskCompleted()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UReloadAbility::OnReloadTaskCompleted)
	
	if (HasAuthority(&CurrentActivationInfo))
	{
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(ReloadEffect,
			GetAbilityLevel(), AbilitySystemComponent->MakeEffectContext());

		const FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

		if (Spec != nullptr)
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
