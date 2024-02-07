// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/SprintAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Framework/AbilityTasks/AbilityTask_WaitStopMoveForward.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


void USprintAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(USprintAbility::ActivateAbility)
	
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (ActorInfo == nullptr || !CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		
		const TWeakObjectPtr<AToyboxCharacter> Character = CastChecked<AToyboxCharacter>(ActorInfo->AvatarActor.Get());
		if (Character.IsValid())
		{
			Character->StartSprint();
		}

		if (bFallingCancelsSprint)
		{
			const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
		
			TagAddedTask = UAbilityTask_WaitGameplayTagAdded::WaitGameplayTagAdd(this, NativeTags.MovementState_Airborne, nullptr, true);
			if (IsValid(TagAddedTask))
			{
				TagAddedTask->Added.AddDynamic(this, &ThisClass::OnAirborneTagAdded);
				TagAddedTask->ReadyForActivation(); //calls activate eventually
			}
		}
		
		StopMoveForwardTask = UAbilityTask_WaitStopMoveForward::WaitStopMoveForward(
			this,
			FName(TEXT("StopMoveForward")),
			ActorInfo->MovementComponent.Get());
		if (IsValid(StopMoveForwardTask))
		{
			//This is currently the neatest solution, because we only need it at this specific place
			StopMoveForwardTask->OnMovementForwardStopped.AddDynamic(this, &ThisClass::OnMoveForwardStopped);
			StopMoveForwardTask->ReadyForActivation();
		}
	}
}

void USprintAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	//if scope is locked, wait until it isnt anymore
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &USprintAbility::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(USprintAbility::CancelAbility)

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	
	if (ActorInfo == nullptr) return;
	const TWeakObjectPtr<AToyboxCharacter> Character = Cast<AToyboxCharacter>(ActorInfo->AvatarActor.Get());
	if (Character.IsValid())
	{
		Character->StopSprint();
	}
}

void USprintAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void USprintAbility::OnAirborneTagAdded()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void USprintAbility::OnMoveForwardStopped()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}
