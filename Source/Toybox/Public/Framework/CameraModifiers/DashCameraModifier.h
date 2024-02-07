// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxCameraModifier.h"
#include "DashCameraModifier.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UDashCameraModifier : public UToyboxCameraModifier
{
	GENERATED_BODY()

public:
	UDashCameraModifier();

protected:
	virtual void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV) override;
};
