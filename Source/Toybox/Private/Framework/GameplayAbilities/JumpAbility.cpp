// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/JumpAbility.h"

UJumpAbility::UJumpAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
}

bool UJumpAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	//Super checks ability Actor Info so we dont have to
	const TWeakObjectPtr<ACharacter> Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	return Character.IsValid() && Character->CanJump();
}

void UJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (ActorInfo == nullptr || !CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			return;
		}
		
		const TWeakObjectPtr<ACharacter> Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		if (Character.IsValid())
		{
			Character->Jump();
		}
	}
}

void UJumpAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void UJumpAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	//if scope is locked, wait until it isnt anymore
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &UJumpAbility::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	
	if (ActorInfo == nullptr) return;
	const TWeakObjectPtr<ACharacter> Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character.IsValid())
	{
		Character->StopJumping();
	}
}
