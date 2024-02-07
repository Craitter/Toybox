// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "FlipCameraAbility.generated.h"

class UOffsetCameraModifier;

/**
 * 
 */
UCLASS()
class TOYBOX_API UFlipCameraAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	UFlipCameraAbility();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	TWeakObjectPtr<UOffsetCameraModifier> GetOffsetModifier(TWeakObjectPtr<APlayerController> Controller);

	FGameplayTag ModifierTag = FGameplayTag::EmptyTag;
};
