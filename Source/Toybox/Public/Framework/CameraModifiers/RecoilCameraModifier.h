// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxCameraModifier.h"
#include "RecoilCameraModifier.generated.h"

UENUM()
enum class ELerpType
{
	Lerp,
	InterpEaseIn,
	InterpEaseOut,
	InterpExpoIn,
	InterpExpoOut,
	InterpSinIn,
	InterpSinOut,
	InterpCircularIn,
	InterpCircularOut,
};

/**
 * 
 */
UCLASS()
class TOYBOX_API URecoilCameraModifier : public UToyboxCameraModifier
{
	GENERATED_BODY()

public:
	URecoilCameraModifier();

	void ApplyRecoil(float Recoil);

	UPROPERTY(EditDefaultsOnly)
	ELerpType LerpType = ELerpType::Lerp;
	
	// Used in some Interp settings
	UPROPERTY(EditDefaultsOnly)
	float Exponential = 0.0f;
	
protected:
	virtual void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV) override;

	float GetLerpValue() const;
	
	float StartingPitch = 0.0f;
	float CurrentPitch = 0.0f;
	float LastPitch = 0.0f;
	float TargetPitch = 0.0f;
};
