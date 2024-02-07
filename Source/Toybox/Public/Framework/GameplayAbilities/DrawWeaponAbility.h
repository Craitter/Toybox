// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "DrawWeaponAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class TOYBOX_API UDrawWeaponAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	UDrawWeaponAbility(); 
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "DrawWeapon")
	TObjectPtr<UAnimMontage> DrawAnimMontage = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "DrawWeapon", Meta = (Categories = "Weapon"))
	FGameplayTag WeaponTag = FGameplayTag::EmptyTag;

	UFUNCTION()
	void OnMontageTaskEnds();

private:
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageAndWaitTask = {nullptr};
};
