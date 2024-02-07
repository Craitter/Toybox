// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CameraModifiers/DashCameraModifier.h"

#include "Actors/ToyboxCharacter.h"
#include "Framework/ToyboxCharacterMovementComponent.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


UDashCameraModifier::UDashCameraModifier()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();

	Tag = NativeTags.CameraModifier_DashCameraMod;
	AlphaInTime = 0.5f;
	AlphaOutTime = 0.5f;
	bDisabled = true;
}

void UDashCameraModifier::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV,
	FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UDashCameraModifier::ModifyCamera)
	
	const TWeakObjectPtr<AToyboxCharacter> Character = Cast<AToyboxCharacter>(GetViewTarget());
	if (!Character.IsValid())
	{
		return;
	}
	const TWeakObjectPtr<UToyboxCharacterMovementComponent> MovementComponent = Character->GetToyboxCharacterMovement();
	if (!MovementComponent.IsValid())
	{
		return;
	}
	
	const float TargetCrouchOffset = MovementComponent->GetCrouchedHalfHeight() - Character->GetDefaultHalfHeight();
	float Offset = FMath::Lerp(0.0f, TargetCrouchOffset, Alpha);
	
	if (MovementComponent->IsCrouching())
	{
		Offset -= TargetCrouchOffset;
	}
	if (MovementComponent->IsMovingOnGround())
	{
		NewViewLocation = ViewLocation;
		NewViewLocation.Z += Offset;
	}
}
