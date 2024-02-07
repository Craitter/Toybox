// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CameraModifiers/FadeCharacterCameraModifier.h"

#include "Actors/MeleeWeapon.h"
#include "Actors/RangedWeapon.h"
#include "Actors/ToyboxCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Toybox/Toybox.h"


constexpr float G_No_Fade_Length = 1.0f;

void UFadeCharacterCameraModifier::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV,
	FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UFadeCharacterCameraModifier::ModifyCamera)
	
	const TWeakObjectPtr<AActor> ViewTarget = GetViewTarget();
	if (!ViewTarget.IsValid() || !IsValid(FadePercentage))
	{
		return;
	}

	const TWeakObjectPtr<USpringArmComponent> SpringArmComponent = ViewTarget->FindComponentByClass<USpringArmComponent>();
	if (!SpringArmComponent.IsValid())
	{
		return;
	}
	if (!SpringArmComponent->IsCollisionFixApplied())
	{
		//This just means we will not fade at all because we are not clipping towards the player
		ApplyFadePercentage(ViewTarget, G_No_Fade_Length);
		return;
	}
	//Getting the Percentage: Current = 300 / Target = 600 = 0.5;
	//How much percent should we fade if we have only 50% of the desired target arm length?
	//The Material Handles the Fading with a start and end distance
	//But we evaluate the Fade amount with the current TargetArmLength compared to the desired, so we have to calculate
	//at which distance we would have 0% fade so the current distance matches the resulting fade percentage
	
	//In this case there is no scenario where we return -1 but usually we need to handle it
	const float CurrentSpringArmLength = GetCurrentSpringArmLength(SpringArmComponent);
	const float RelativeCurrentArmLength = CurrentSpringArmLength / SpringArmComponent->TargetArmLength;
	const float FadePercentageToApply = FadePercentage->FloatCurve.Eval(RelativeCurrentArmLength);
	const float ResultingStartFadeDistance = CurrentSpringArmLength / FadePercentageToApply;

	ApplyFadePercentage(ViewTarget, ResultingStartFadeDistance);
}

void UFadeCharacterCameraModifier::ApplyFadePercentage(const TWeakObjectPtr<AActor> ViewTarget, const float StartToFadeLength)
{
	if (!ViewTarget.IsValid())
	{
		return;
	}
	TArray<UMeshComponent*> MeshComponents;
	ViewTarget->GetComponents<UMeshComponent>(MeshComponents);
	GetWeaponMeshes(MeshComponents, ViewTarget);
	SetStartFadeDistanceForMeshComponents(MeshComponents, StartToFadeLength);
}

void UFadeCharacterCameraModifier::SetStartFadeDistanceForMeshComponents(const TArray<UMeshComponent*>& MeshComponents,
	const float StartFadeDistance)
{
	for (const auto MeshComponent : MeshComponents)
	{
		if (IsValid(MeshComponent))
		{
			MeshComponent->SetScalarParameterValueOnMaterials(TEXT("EndFadeDistanceFromCamera"), StartFadeDistance);
		}
	}
}

float UFadeCharacterCameraModifier::GetCurrentSpringArmLength(const TWeakObjectPtr<USpringArmComponent> SpringArmComponent)
{
	if (!SpringArmComponent.IsValid())
	{
		return -1.0f;
	}
	const FVector DesiredCameraLocation = SpringArmComponent->GetUnfixedCameraPosition();
	const FVector CurrentCameraLocation = SpringArmComponent->GetSocketTransform(SpringArmComponent->SocketName, RTS_World).GetLocation();
	const float CameraDistanceToDesiredLocation = FVector::Distance(DesiredCameraLocation, CurrentCameraLocation);
	return SpringArmComponent->TargetArmLength - CameraDistanceToDesiredLocation;
}

void UFadeCharacterCameraModifier::GetWeaponMeshes(TArray<UMeshComponent*>& MeshArray, const TWeakObjectPtr<AActor> ViewTarget)
{
	const TWeakObjectPtr<AToyboxCharacter> Character = Cast<AToyboxCharacter>(ViewTarget);

	if (!Character.IsValid())
	{
		return;
	}
	
	const TWeakObjectPtr<AMeleeWeapon> MeleeWeapon = Character->GetEquippedMelee();
	MeshArray.Add(MeleeWeapon->GetWeaponMesh().Get());

	const TWeakObjectPtr<ARangedWeapon> RangedWeapon = Character->GetEquippedRanged();
	MeshArray.Add(RangedWeapon->GetWeaponMesh().Get());
}
