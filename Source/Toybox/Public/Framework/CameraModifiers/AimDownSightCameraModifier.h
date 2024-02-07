// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxCameraModifier.h"
#include "AimDownSightCameraModifier.generated.h"

class AToyboxPlayerController;

/**
 * 
 */
UCLASS()
class TOYBOX_API UAimDownSightCameraModifier : public UToyboxCameraModifier
{
	GENERATED_BODY()

public:
	UAimDownSightCameraModifier();
	
	virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;

	virtual void EnableModifier() override;

	virtual void DisableModifier(bool bImmediate = false) override;

private:
	const TWeakObjectPtr<AToyboxPlayerController> Controller = {nullptr};

	UPROPERTY(Category = Custom, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = 0.0f, ClampMax = 100000.0f))
	float MaxTargetArmLength = 400.0f;

	UPROPERTY(Category = Custom, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = 0.0f, ClampMax = 100000.0f))
	float ADSTargetArmLength = 200.0f;
};
