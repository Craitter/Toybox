// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "GenericCooldownModCalc.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UGenericCooldownModCalc : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
