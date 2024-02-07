// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CameraModifiers/AimDownSightCameraModifier.h"
#include "GameFramework/SpringArmComponent.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"
#include "Toybox/Public/Actors/ToyboxPlayerController.h"


UAimDownSightCameraModifier::UAimDownSightCameraModifier()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	Tag = NativeTags.AbilityTag_Weapon_AimDownSight;
	bDisabled = true;
}

bool UAimDownSightCameraModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAimDownSightCameraModifier::ModifyCamera)

	
	Super::ModifyCamera(DeltaTime, InOutPOV);

	if (!SpringArm.IsValid())
	{
		DisableModifier();
		return false;
	}

	SpringArm->TargetArmLength = ADSTargetArmLength;

	return false;
}

void UAimDownSightCameraModifier::EnableModifier()
{
	if (SetSpringArm())
	{
		Super::EnableModifier();
	}
}

void UAimDownSightCameraModifier::DisableModifier(bool bImmediate)
{
	Super::DisableModifier(bImmediate);

	if (!SpringArm.IsValid())
	{
		return;
	}

	SpringArm->TargetArmLength = MaxTargetArmLength;
}