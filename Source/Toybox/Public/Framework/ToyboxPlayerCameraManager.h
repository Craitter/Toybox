// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Camera/PlayerCameraManager.h"
#include "ToyboxPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API AToyboxPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	TWeakObjectPtr<UCameraModifier> FindCameraModifierByTag(const FGameplayTag& TagToCheck);
};
