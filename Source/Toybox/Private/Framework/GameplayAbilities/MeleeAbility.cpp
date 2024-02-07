// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/MeleeAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Actors/MeleeWeapon.h"
#include "Framework/AnimationNotifies/MeleeAnimNotifyState.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


UMeleeAbility::UMeleeAbility()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	DamageModiferTag = NativeTags.MetaTag_Damage;

	ComboFailDelayTimer.Invalidate();

	bServerRespectsRemoteAbilityCancellation = false;
}

void UMeleeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMeleeAbility::ActivateAbility)
	
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) || ActorInfo == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	Character = CastChecked<AToyboxCharacter>(ActorInfo->AvatarActor.Get());

	AbilitySystemComponent = Character->GetAbilitySystemComponent();

	if (!Character.IsValid() || !AbilitySystemComponent.IsValid() || !IsValid(DealDamageGameplayEffect))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	AnimNotifyEventHandle = Character->OnAnimNotifyEvent.AddUObject(this, &ThisClass::OnMeleeNotify);

	const TWeakObjectPtr<AMeleeWeapon> Weapon = Character->GetEquippedMelee();

	if (!Weapon.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	ListOfAttacks = Weapon->GetAttackList();

	AbilityInputPressedTask = UAbilityTask_WaitInputPress::WaitInputPress(this, false);
	
	if (!IsValid(AbilityInputPressedTask))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	AbilityInputPressedTask->OnPress.AddDynamic(this, &ThisClass::OnAbilityInputPressed);

	AbilityInputPressedTask->ReadyForActivation();
	
	if (!StartAttack())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
}

void UMeleeAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	if (ScopeLockCount > 0)
	{
		UE_LOG(LogAbilitySystem, Verbose, TEXT("Attempting to cancel Ability %s but ScopeLockCount was greater than 0, adding cancel to the WaitingToExecute Array"), *GetName());
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}
	
	// if (IsValid(PlayMontageAndWaitTask))
	// {
	// 	PlayMontageAndWaitTask->ExternalCancel();
	// }
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UMeleeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Reset all variables before starting ability logic
	CurrentAttackIndex = 0;
	bCanCombo = false;
	AlreadyHitActors.Reset();

	if (Character.IsValid())
	{
		Character->OnAnimNotifyEvent.Remove(AnimNotifyEventHandle);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMeleeAbility::OnMontageTaskEnds()
{
	// UE_LOG(LogToyboxAbilitySystem, Warning, TEXT("Animation ended. StartNextCombo = %s"),
	// 	bStartedNextCombo ? TEXT("True") : TEXT("False"))

	if (bCanCombo && ComboFailDelay > UE_FLOAT_NORMAL_THRESH)
	{
		check(GetWorld());

		GetWorld()->GetTimerManager().SetTimer(ComboFailDelayTimer, this, &ThisClass::OnComboFailed, ComboFailDelay, false);

		return;
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMeleeAbility::OnMontageTaskCancelled()
{
	CancelAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, true);
}

void UMeleeAbility::OnComboFailed()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMeleeAbility::SetCanCombo(bool bNewValue)
{
	bCanCombo = bNewValue;
}

void UMeleeAbility::OnAbilityInputPressed(float TimeWaited)
{
	/*UE_LOG(LogToyboxAbilitySystem, Warning, TEXT("Input pressed. bCanCombo = %s, Content string = %s"),
		  bCanCombo ? TEXT("True") : TEXT("False"), *GetClientServerContextString())*/

	TRACE_CPUPROFILER_EVENT_SCOPE(UMeleeAbility::OnAbilityInputPressed)

	check(GetWorld());
	
	GetWorld()->GetTimerManager().ClearTimer(ComboFailDelayTimer);
	
	if (bCanCombo)
	{		
		if (++CurrentAttackIndex < ListOfAttacks.Num())
		{
			StartAttack();
		}
	}

	// We are only allowing you to try to combo once (for now!)
	bCanCombo = false;
}

void UMeleeAbility::OnMeleeNotify(const FName NotifyName, IAnimNotifyInterface* AnimNotifyInterface)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMeleeAbility::OnMeleeNotify)

	if (NotifyName == AnimNotifyNames::MeleeNotify)
	{
		if (AnimNotifyInterface != nullptr)
		{
			FAnimNotifyData NotifyData;
			if (AnimNotifyInterface->GetAnimNotifyData(Character->GetMesh(), NotifyData))
			{
				MeleeHit(NotifyData.HitResultList);
			}
		}
	}
	else if (NotifyName == AnimNotifyNames::MeleeComboStartNotify)
	{
		SetCanCombo(true);
	}
	else if (NotifyName == AnimNotifyNames::MeleeComboEndNotify)
	{
		SetCanCombo(false);
	}
}

void UMeleeAbility::MeleeHit(const TArray<FHitResult>& HitResults)
{
	for (const FHitResult& HitResult : HitResults)
	{
		if (AlreadyHitActors.Contains(HitResult.GetActor()))
		{
			continue;
		}
		
		const IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(HitResult.GetActor());

		if (AbilityInterface != nullptr)
		{
			DealDamage(AbilityInterface->GetAbilitySystemComponent(), HitResult);
			AlreadyHitActors.Add(HitResult.GetActor());
		}
	}
}

void UMeleeAbility::DealDamage(const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent, const FHitResult& HitResult) const
{
	if (!AbilitySystemComponent.IsValid() || !IsValid(DealDamageGameplayEffect))
	{
		return;
	}

	if (!TargetAbilitySystemComponent.IsValid())
	{
		return;
	}
	
	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DealDamageGameplayEffect,
		GetAbilityLevel(), AbilitySystemComponent->MakeEffectContext());

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	if (Spec != nullptr)
	{
		Spec->SetSetByCallerMagnitude(DamageModiferTag, ListOfAttacks[CurrentAttackIndex].Damage);
		Spec->GetContext().AddHitResult(HitResult);
		
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*Spec, TargetAbilitySystemComponent.Get());
	}
}

bool UMeleeAbility::StartAttack()
{
	if (!ListOfAttacks.IsValidIndex(CurrentAttackIndex))
	{
		UE_LOG(LogToyboxAbilitySystem, Error, TEXT("No Attack animation found for Melee ability."))
		return false;
	}

	AlreadyHitActors.Reset();

	PlayMontageAndWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("MeleeAttackMontageTask"),
			ListOfAttacks[CurrentAttackIndex].Animation.Get());
		
	if (!IsValid(PlayMontageAndWaitTask))
	{
		return false;
	}

	PlayMontageAndWaitTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageTaskCancelled);
	PlayMontageAndWaitTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageTaskEnds);
	
	PlayMontageAndWaitTask->ReadyForActivation();

	return true;
}
