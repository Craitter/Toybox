// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/GameplayAbilities/SpecialAbility.h"
#include "MilitaryZeal.generated.h"

class UAbilityTask_WaitTargetData;
/**
 * 
 */
UCLASS()
class TOYBOX_API UMilitaryZeal : public USpecialAbility
{
	GENERATED_BODY()

public:
	UMilitaryZeal();

	FGameplayTag MetaHealTag;
	
protected:
	virtual void ExecuteAttacker(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ExecuteDefender(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly)
	float AttackerRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float DefenderRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float DefenderHealPerAttacker = 30.0f;

	UPROPERTY(EditDefaultsOnly)
	float AttackerHeal = 100.0f;
};
