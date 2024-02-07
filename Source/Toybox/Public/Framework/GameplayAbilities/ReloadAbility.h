// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "ReloadAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class TOYBOX_API UReloadAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	UReloadAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


	UPROPERTY(EditDefaultsOnly, Category="Attacks")
	TSubclassOf<UGameplayEffect> ReloadEffect = {nullptr};
protected:
	UFUNCTION()
	void OnReloadTaskFailed();

	UFUNCTION()
	void OnReloadTaskCompleted();

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageAndWaitTask = {nullptr};
	
	TWeakObjectPtr<ARangedWeapon> Weapon = {nullptr};

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = {nullptr};
};
