// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DamageGameplayEffectExecCalc.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UDamageGameplayEffectExecCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UDamageGameplayEffectExecCalc();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
