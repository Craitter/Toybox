// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxCameraModifier.h"
#include "OffsetCameraModifier.generated.h"

class AToyboxPlayerController;

/**
 * 
 */
UCLASS()
class TOYBOX_API UOffsetCameraModifier : public UToyboxCameraModifier
{
	GENERATED_BODY()

public:
	UOffsetCameraModifier();
	
	virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;
	
	void FlipOffset();

private:
	bool bIsFlipped = false;
	
	// Curve to convert changes in distance to changes in camera Y Pos
	UPROPERTY(Category = Custom, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> DistanceToOffsetYCurve = {nullptr};

	// Curve to convert changes in distance to changes in camera Z Pos
	UPROPERTY(Category = Custom, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> DistanceToOffsetZCurve = {nullptr};
	
	float GetArmLength() const;

	FVector2D GetOffset(float ArmLength) const;
};
