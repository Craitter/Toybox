// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/CameraModifiers/ToyboxCameraModifier.h"
#include "GameFramework/SpringArmComponent.h"

bool UToyboxCameraModifier::HasTag(FGameplayTag TagToCheck) const
{
	if (Tag == TagToCheck)
	{
		return true;
	}
	return false;
}

bool UToyboxCameraModifier::SetSpringArm()
{
	SpringArm = GetViewTarget()->FindComponentByClass<USpringArmComponent>();

	return SpringArm.IsValid();
}