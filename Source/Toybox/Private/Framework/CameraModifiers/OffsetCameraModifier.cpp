// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CameraModifiers/OffsetCameraModifier.h"
#include "GameFramework/SpringArmComponent.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


UOffsetCameraModifier::UOffsetCameraModifier()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	Tag = NativeTags.AbilityTag_Camera_FlipOffset;
}

bool UOffsetCameraModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UOffsetCameraModifier::ModifyCamera)

	Super::ModifyCamera(DeltaTime, InOutPOV);

	if (!DistanceToOffsetYCurve || !DistanceToOffsetZCurve)
	{
		UE_LOG(LogTemp, Error, TEXT("Either DistanceToOffsetYCurve or DistanceToOffsetZCurve is not defined in OffsetCameraModifier"))
		DisableModifier();
		return false;
	}
	
	if (!SpringArm.IsValid())
	{
		if (!SetSpringArm())
		{
			return false;
		}
	}
	
	const float ArmLength = GetArmLength();

	const FVector2D Offset = GetOffset(ArmLength);
	
	// Forcing first value to 0 as this is controlled by the spring arm target length
	SpringArm->SocketOffset = FVector(0.0f, Offset.X, Offset.Y);

	return false;
}

void UOffsetCameraModifier::FlipOffset()
{
	bIsFlipped = !bIsFlipped;
}

float UOffsetCameraModifier::GetArmLength() const
{
	// Forcing rotation back to 0 so we can more easily find the arm length
	// Setting Yaw to 0 as we dont care about it!
	
	FRotator SpringRotation = SpringArm->GetTargetRotation() * -1.0f;
	SpringRotation.Yaw = 0.0f;

	const FTransform& CameraRelativeTransform = SpringArm->GetSocketTransform(SpringArm->SocketName, RTS_Component);
	const FVector& CameraRelativePosition = CameraRelativeTransform.GetLocation();
	const FVector& CameraRelativeVector = SpringRotation.RotateVector(CameraRelativePosition);
	
	return fabs(CameraRelativeVector.X);
}

FVector2D UOffsetCameraModifier::GetOffset(float ArmLength) const
{
	FVector2D Offset(DistanceToOffsetYCurve->GetFloatValue(ArmLength), DistanceToOffsetZCurve->GetFloatValue(ArmLength));
	if (bIsFlipped)
	{
		Offset.X *= -1.0f;
	}
	return Offset;
}
