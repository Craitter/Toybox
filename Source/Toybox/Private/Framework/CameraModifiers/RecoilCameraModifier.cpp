// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CameraModifiers/RecoilCameraModifier.h"

#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


URecoilCameraModifier::URecoilCameraModifier()
{
	AlphaOutTime = 0.0f;

	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	Tag = NativeTags.AbilityTag_Weapon_Recoil;
}

void URecoilCameraModifier::ApplyRecoil(const float Recoil)
{
	Alpha = 0.0f;
	
	const FRotator CurrentRotation = CameraOwner->GetOwningPlayerController()->GetControlRotation();
	StartingPitch = FRotator::NormalizeAxis(CurrentRotation.Pitch);

	TargetPitch = FMath::Clamp(FRotator::NormalizeAxis(CurrentRotation.Pitch) + Recoil,
		CameraOwner->ViewPitchMin, CameraOwner->ViewPitchMax);

	LastPitch = StartingPitch;
	
	if (IsDisabled())
	{
		EnableModifier();
	}
}

void URecoilCameraModifier::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV,
	FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URecoilCameraModifier::ModifyCamera)

	CurrentPitch = GetLerpValue();
	
	const float DeltaPitch = CurrentPitch - LastPitch;
	
	CameraOwner->GetOwningPlayerController()->AddPitchInput(-DeltaPitch);

	LastPitch = CurrentPitch;
	
	if (Alpha >= (1.0f - UE_FLOAT_NORMAL_THRESH))
	{
		DisableModifier(true);
	}
}

float URecoilCameraModifier::GetLerpValue() const
{
	if (LerpType == ELerpType::Lerp)
	{
		return FMath::Lerp(StartingPitch, TargetPitch, Alpha);
	}
	if (LerpType == ELerpType::InterpEaseIn)
	{
		return FMath::InterpEaseIn(StartingPitch, TargetPitch, Alpha, Exponential);
	}
	if (LerpType == ELerpType::InterpEaseOut)
	{
		return FMath::InterpEaseOut(StartingPitch, TargetPitch, Alpha, Exponential);
	}
	if (LerpType == ELerpType::InterpExpoIn)
	{
		return FMath::InterpExpoIn(StartingPitch, TargetPitch, Alpha);
	}
	if (LerpType == ELerpType::InterpExpoOut)
	{
		return FMath::InterpExpoOut(StartingPitch, TargetPitch, Alpha);
	}
	if (LerpType == ELerpType::InterpSinIn)
	{
		return FMath::InterpSinIn(StartingPitch, TargetPitch, Alpha);
	}
	if (LerpType == ELerpType::InterpSinOut)
	{
		return FMath::InterpSinOut(StartingPitch, TargetPitch, Alpha);
	}
	if (LerpType == ELerpType::InterpCircularIn)
	{
		return FMath::InterpCircularIn(StartingPitch, TargetPitch, Alpha);
	}
	if (LerpType == ELerpType::InterpCircularOut)
	{
		return FMath::InterpCircularOut(StartingPitch, TargetPitch, Alpha);
	}
	
	return 0.0f;
}
