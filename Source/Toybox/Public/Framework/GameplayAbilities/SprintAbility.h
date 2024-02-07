// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "SprintAbility.generated.h"

class UAbilityTask_WaitStopMoveForward;
class UAbilityTask_WaitInputPress;
class UAbilityTask_WaitGameplayTagAdded;
/**
 * 
 */
UCLASS()
class TOYBOX_API USprintAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	bool bFallingCancelsSprint = false;
	
private:
	UFUNCTION()
	void OnAirborneTagAdded();

	UFUNCTION()
	void OnMoveForwardStopped();
	
	TObjectPtr<UAbilityTask_WaitGameplayTagAdded> TagAddedTask = {nullptr};
	
	TObjectPtr<UAbilityTask_WaitStopMoveForward> StopMoveForwardTask = {nullptr};
};
