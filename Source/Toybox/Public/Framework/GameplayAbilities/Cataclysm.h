// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/GameplayAbilities/SpecialAbility.h"
#include "Cataclysm.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UCataclysm : public USpecialAbility
{
	GENERATED_BODY()

public:
	UCataclysm();

	FGameplayTag MetaDamageTag = FGameplayTag::EmptyTag;
	
protected:
	virtual void ExecuteAttacker(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ExecuteDefender(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly)
	float AttackerRadius = 2000.0f;

	UPROPERTY(EditDefaultsOnly)
	float DefenderRadius = 2000.0f;

	UPROPERTY(EditDefaultsOnly)
	float DamageAsDefender = 20.0f;
	
	UPROPERTY(EditDefaultsOnly)
	float DamageAsAttacker = 20.0f;
	
	UPROPERTY(EditDefaultsOnly)
	bool bIgnoreBlockingTerrain = false;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "!bIgnoreBlockingTerrain"))
	float RadiusThatAlwaysHits = 100.0f;
	

	
};
