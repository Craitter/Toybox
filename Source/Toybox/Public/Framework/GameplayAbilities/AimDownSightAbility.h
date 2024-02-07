// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "AimDownSightAbility.generated.h"

class UAimDownSightCameraModifier;
/**
 * 
 */
UCLASS()
class TOYBOX_API UAimDownSightAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	UAimDownSightAbility();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

private:
	TWeakObjectPtr<UAimDownSightCameraModifier> GetAimDownSightModifer(TWeakObjectPtr<APlayerController> Controller);

	FGameplayTag ModifierTag = FGameplayTag::EmptyTag;
};
