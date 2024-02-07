// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "WeaponGameplayCue.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UWeaponGameplayCue : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()
	
protected:
	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;
};
