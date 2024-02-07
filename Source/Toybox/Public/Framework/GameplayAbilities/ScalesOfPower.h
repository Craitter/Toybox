// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/GameplayAbilities/SpecialAbility.h"
#include "ScalesOfPower.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UScalesOfPower : public USpecialAbility
{
	GENERATED_BODY()

public:
	UScalesOfPower();

	FGameplayTag MetaDurationTag = FGameplayTag::EmptyTag;
	FGameplayTag MetaAbsoluteDamageBuffTag = FGameplayTag::EmptyTag;
	
protected:
	virtual void ExecuteAttacker(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ExecuteDefender(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;


	UPROPERTY(EditDefaultsOnly)
	float AttackerRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float AttackerAbsoluteDamageBuff = 100.0f;

	UPROPERTY(EditDefaultsOnly)
	float AttackerDuration = 10.0f;
	
	UPROPERTY(EditDefaultsOnly)
	float DefenderDuration = 10.0f;

	UPROPERTY(EditDefaultsOnly)
	float DefenderRadius = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float DefenderAbsoluteDamageBuffPerAttacker = 30.0f;
	
};
