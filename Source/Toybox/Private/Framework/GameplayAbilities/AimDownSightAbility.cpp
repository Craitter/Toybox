// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/AimDownSightAbility.h"

#include "AbilitySystemLog.h"
#include "Framework/ToyboxPlayerCameraManager.h"
#include "Framework/CameraModifiers/AimDownSightCameraModifier.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


UAimDownSightAbility::UAimDownSightAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	ModifierTag = NativeTags.AbilityTag_Weapon_AimDownSight;
}

void UAimDownSightAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAimDownSightAbility::ActivateAbility)
	
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (ActorInfo == nullptr || !CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		
		const TWeakObjectPtr<APlayerController> Controller = ActorInfo->PlayerController.Get();
		
		if (!Controller.IsValid())
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}

		const TWeakObjectPtr<UAimDownSightCameraModifier> CameraModifer = GetAimDownSightModifer(Controller);

		if (!CameraModifer.IsValid())
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		
		CameraModifer->EnableModifier();
	}
}

void UAimDownSightAbility::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void UAimDownSightAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle,
 const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
 bool bReplicateCancelAbility)
{
	if (ScopeLockCount > 0)
	{
		UE_LOG(LogAbilitySystem, Verbose, TEXT("Attempting to cancel Ability %s but ScopeLockCount was greater than 0, adding cancel to the WaitingToExecute Array"), *GetName());
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &UAimDownSightAbility::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}
	TRACE_CPUPROFILER_EVENT_SCOPE(UAimDownSightAbility::CancelAbility)

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	
	const TWeakObjectPtr<APlayerController> Controller = ActorInfo->PlayerController.Get();
		
	if (!Controller.IsValid())
	{
		return;
	}

	const TWeakObjectPtr<UAimDownSightCameraModifier> CameraModifer = GetAimDownSightModifer(Controller);

	if (!CameraModifer.IsValid())
	{
		return;
	}

	CameraModifer->DisableModifier(true);
}

TWeakObjectPtr<UAimDownSightCameraModifier> UAimDownSightAbility::GetAimDownSightModifer(TWeakObjectPtr<APlayerController> Controller)
{
	const TWeakObjectPtr<AToyboxPlayerCameraManager> CameraManager = Cast<AToyboxPlayerCameraManager>(Controller->PlayerCameraManager);

	if (!CameraManager.IsValid())
	{
		return nullptr;
	}
	
	using ADSModiferClass = UAimDownSightCameraModifier;
	const TWeakObjectPtr<ADSModiferClass> Modifier = Cast<ADSModiferClass>(CameraManager->
		FindCameraModifierByTag(ModifierTag));
	return Modifier;
}
