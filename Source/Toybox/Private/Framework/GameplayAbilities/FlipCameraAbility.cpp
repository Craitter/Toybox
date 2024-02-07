// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/FlipCameraAbility.h"

#include "Framework/ToyboxPlayerCameraManager.h"
#include "Framework/CameraModifiers/OffsetCameraModifier.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


UFlipCameraAbility::UFlipCameraAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	ModifierTag = NativeTags.AbilityTag_Camera_FlipOffset;
}

void UFlipCameraAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UFlipCameraAbility::ActivateAbility)

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

		const TWeakObjectPtr<UOffsetCameraModifier> CameraModifer = GetOffsetModifier(Controller);

		if (!CameraModifer.IsValid())
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		
		CameraModifer->FlipOffset();
		
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

TWeakObjectPtr<UOffsetCameraModifier> UFlipCameraAbility::GetOffsetModifier(TWeakObjectPtr<APlayerController> Controller)
{
	const TWeakObjectPtr<AToyboxPlayerCameraManager> CameraManager = Cast<AToyboxPlayerCameraManager>(
		Controller->PlayerCameraManager);

	if (!CameraManager.IsValid())
	{
		return nullptr;
	}
	
	using OffsetModiferClass = UOffsetCameraModifier;
	const TWeakObjectPtr<OffsetModiferClass> Modifier = Cast<OffsetModiferClass>(CameraManager->
		FindCameraModifierByTag(ModifierTag));
	return Modifier;
}

