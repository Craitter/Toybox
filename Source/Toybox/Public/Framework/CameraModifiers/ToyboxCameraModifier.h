// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Camera/CameraModifier.h"
#include "ToyboxCameraModifier.generated.h"

class USpringArmComponent;
/**
 * 
 */
UCLASS()
class TOYBOX_API UToyboxCameraModifier : public UCameraModifier
{
	GENERATED_BODY()

public:
	bool HasTag(FGameplayTag TagToCheck) const;

protected:
	FGameplayTag Tag = FGameplayTag::EmptyTag;

	// This needs to be set by SetSpringArm() otherwise it will be null!
	TWeakObjectPtr<USpringArmComponent> SpringArm = {nullptr};

	// This will return false if it cant find the spring arm
	bool SetSpringArm();
};
