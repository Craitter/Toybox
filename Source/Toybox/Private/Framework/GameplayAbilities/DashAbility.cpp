// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/DashAbility.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraModifier.h"
#include "Framework/ToyboxCharacterMovementComponent.h"
#include "Framework/ToyboxPlayerCameraManager.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


UDashAbility::UDashAbility()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();

	DashModiferTag = NativeTags.CameraModifier_DashCameraMod;
}

void UDashAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UDashAbility::ActivateAbility)

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		FVector DashDirection = FVector::ZeroVector;
		const TWeakObjectPtr<UAnimMontage> DashMontageToPlay = SelectDirectionalAnimMontage(
			GetFacingDirection(), GetAccelerationDirection(), DashDirection);
		
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		PlayDashMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("DashMontage"), DashMontageToPlay.Get());

		RootMotionConstantForceTask = CreateRootMotionTask(DashDirection);
		if (!IsValid(PlayDashMontageTask) || !IsValid(RootMotionConstantForceTask))
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		PlayDashMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnDashMontageCancel);
		PlayDashMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnDashMontageCancel);
		RootMotionConstantForceTask->OnFinish.AddDynamic(this, &ThisClass::OnRootMotionFinished);
		
		PlayDashMontageTask->ReadyForActivation();
		RootMotionConstantForceTask->ReadyForActivation();
		
		StartDashCameraModifier();
		StartDashState();		
	}
}

void UDashAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	//if scope is locked, wait until it isnt anymore
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(
			this, &UDashAbility::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		return;
	}
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UDashAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UDashAbility::EndAbility)

	//if scope is locked, wait until it isnt anymore
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(
			this, &UDashAbility::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
		return;
	}
	EndDashCameraModifier();
	EndDashState();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDashAbility::OnRootMotionFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDashAbility::OnDashMontageCancel()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

FVector UDashAbility::GetFacingDirection() const
{
	const TWeakObjectPtr<AToyboxCharacter> Character = GetToyboxCharacterFromActorInfo();
	if (!Character.IsValid())
	{
		return FVector::ZeroVector;
	}
	return Character->GetActorForwardVector().GetSafeNormal2D();
}

FVector UDashAbility::GetAccelerationDirection() const
{
	const TWeakObjectPtr<AToyboxCharacter> Character = GetToyboxCharacterFromActorInfo();
	if (!Character.IsValid())
	{
		return FVector::ZeroVector;
	}
	const TWeakObjectPtr<UCharacterMovementComponent> MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent.IsValid())
	{
		return FVector::ZeroVector;
	}

	return MovementComponent->GetCurrentAcceleration().GetSafeNormal2D();
}

FVector UDashAbility::GetCameraDirection() const
{
	const TWeakObjectPtr<AToyboxCharacter> Character = GetToyboxCharacterFromActorInfo();
	if (!Character.IsValid())
	{
		return FVector::ZeroVector;
	}
	const TWeakObjectPtr<UCameraComponent> Camera = Character->FindComponentByClass<UCameraComponent>();
	if (!Camera.IsValid())
	{
		return FVector::ZeroVector;
	}
	return Camera->GetForwardVector().GetSafeNormal2D();
}

void UDashAbility::StartDashCameraModifier() const
{
	const TWeakObjectPtr<APlayerController> Controller = GetToyboxPlayerControllerFromActorInfo();
	if (Controller.IsValid())
	{
		const TWeakObjectPtr<AToyboxPlayerCameraManager> CameraManager =
			Cast<AToyboxPlayerCameraManager>(Controller->PlayerCameraManager);
		if (CameraManager.IsValid())
		{
			const TWeakObjectPtr<UCameraModifier> DashModifier = CameraManager->FindCameraModifierByTag(DashModiferTag);
			if (DashModifier.IsValid())
			{
				DashModifier->EnableModifier();
			}
		}
	}	
}

void UDashAbility::EndDashCameraModifier() const
{
	const TWeakObjectPtr<APlayerController> Controller = GetToyboxPlayerControllerFromActorInfo();
	if (Controller.IsValid())
	{
		const TWeakObjectPtr<AToyboxPlayerCameraManager> CameraManager =
			Cast<AToyboxPlayerCameraManager>(Controller->PlayerCameraManager);
		if (CameraManager.IsValid())
		{
			const TWeakObjectPtr<UCameraModifier> DashModifier = CameraManager->FindCameraModifierByTag(DashModiferTag);
			if (DashModifier.IsValid())
			{
				DashModifier->DisableModifier();
			}
		}
	}	
}

void UDashAbility::StartDashState() const
{
	if (GetCurrentActorInfo() != nullptr)
	{
		const TWeakObjectPtr<UToyboxCharacterMovementComponent> MovementComponent =
			Cast<UToyboxCharacterMovementComponent>(GetCurrentActorInfo()->MovementComponent);
		if (MovementComponent.IsValid())
		{
			MovementComponent->DashStarted();
		}
	}
}

void UDashAbility::EndDashState() const
{
	if (GetCurrentActorInfo() != nullptr)
	{
		const TWeakObjectPtr<UToyboxCharacterMovementComponent> MovementComponent =
			Cast<UToyboxCharacterMovementComponent>(GetCurrentActorInfo()->MovementComponent);
		if (MovementComponent.IsValid())
		{
			MovementComponent->DashEnded();
		}
	}
}


TObjectPtr<UAbilityTask_ApplyRootMotionConstantForce> UDashAbility::CreateRootMotionTask(const FVector& DashDirection)
{
	const TWeakObjectPtr<AToyboxCharacter> Character = GetToyboxCharacterFromActorInfo();
	if (!Character.IsValid())
	{
		return nullptr;
	}
	const TWeakObjectPtr<UToyboxCharacterMovementComponent> MovementComponent = Character->GetToyboxCharacterMovement();
	if (!MovementComponent.IsValid())
	{
		return nullptr;
	}
	
	return UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this, FName(TEXT("DashTask")), DashDirection,
			MovementComponent->DashStrength,
			MovementComponent->DashDuration,
			MovementComponent->bDashIsAdditive,
			MovementComponent->DashStrengthOverTime,
			ERootMotionFinishVelocityMode::ClampVelocity,
			FVector::ZeroVector,
			MovementComponent->MaxWalkSpeed,
			MovementComponent->bEnableGravity);
}

TWeakObjectPtr<UAnimMontage> UDashAbility::SelectDirectionalAnimMontage(const FVector& FacingDirection,
	const FVector& AccelerationDirection, FVector& DashDirection) const
{
	TWeakObjectPtr<UAnimMontage> OutMontage = nullptr;
	bool bBiasForward = false;
	const FRotator FacingRotation = FacingDirection.ToOrientationRotator();
	const FRotator AccelerationRotation = AccelerationDirection.ToOrientationRotator();

	const FRotator DeltaRotation = (AccelerationRotation - FacingRotation).GetNormalized();
	const float DeltaYawRotation = DeltaRotation.Yaw;
	const float AbsoluteDeltaYawRotation = fabs(DeltaYawRotation);
	if (AbsoluteDeltaYawRotation < 45.0f) // Normalized Rotation = [-180, 180], absolute is then 180, < 45 means its in the forward 90 degree
	{
		bBiasForward = true;
		OutMontage = DashForwardMontage; //Fwd
	}
	else if (AbsoluteDeltaYawRotation > 135.0f) // same as above just that these are the backwards 90 degree
	{
		OutMontage = DashBackwardMontage; //Back
	}
	else if (DeltaYawRotation < 0.0f) //its not forward and backward, if we are having a negative value we know it has to be on the left side
	{
		OutMontage = DashLeftMontage; //Left
	}
	else //okay, it has to be the right side
	{
		OutMontage = DashRightMontage; //Right
	}

	if (bBiasForward)
	{
		DashDirection = GetCameraDirection();
	}
	else
	{
		DashDirection = AccelerationDirection;
	}
	if (DashDirection.IsNearlyZero())
	{
		DashDirection = FacingDirection;
	}
	return OutMontage;
}
