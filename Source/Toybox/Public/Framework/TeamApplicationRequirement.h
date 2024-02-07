// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectCustomApplicationRequirement.h"
#include "TeamApplicationRequirement.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UTeamApplicationRequirement : public UGameplayEffectCustomApplicationRequirement
{
	GENERATED_BODY()

	virtual bool CanApplyGameplayEffect_Implementation(const UGameplayEffect* GameplayEffect, const FGameplayEffectSpec& Spec, UAbilitySystemComponent* ASC) const override;

private:
	static bool IsFriendly(TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent, TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent);
	
	static bool IsDefenders(TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent, TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent);
	static bool IsAttackers(TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent, TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent);
};
