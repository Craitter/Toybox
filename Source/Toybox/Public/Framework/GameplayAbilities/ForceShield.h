// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/GameplayAbilities/SpecialAbility.h"
#include "ForceShield.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UForceShield : public USpecialAbility
{
	GENERATED_BODY()

public:
	UForceShield();

	FGameplayTag MetaShieldTag;

protected:
	virtual void ExecuteAttacker(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ExecuteDefender(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly)
	float AttackerRadius = 2000.0f;
	
	UPROPERTY(EditDefaultsOnly)
	float DefenderShield = 200.0f;

	UPROPERTY(EditDefaultsOnly)
	float AttackerShield = 100.0f;
};
