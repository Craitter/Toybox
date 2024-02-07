// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxCameraModifier.h"
#include "GameFramework/SpringArmComponent.h"
#include "FadeCharacterCameraModifier.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UFadeCharacterCameraModifier : public UToyboxCameraModifier
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCurveFloat> FadePercentage = {nullptr};

	virtual void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV) override;
	
private:
	static void ApplyFadePercentage(TWeakObjectPtr<AActor> ViewTarget, float StartToFadeLength);
	static void SetStartFadeDistanceForMeshComponents(const TArray<UMeshComponent*>& MeshComponents, float StartFadeDistance);
	static float GetCurrentSpringArmLength(TWeakObjectPtr<USpringArmComponent> SpringArmComponent);
	static void GetWeaponMeshes(TArray<UMeshComponent*>& MeshArray, const TWeakObjectPtr<AActor> ViewTarget);
};
