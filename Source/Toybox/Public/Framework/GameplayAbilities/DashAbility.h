// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "DashAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_ApplyRootMotionConstantForce;
/**
 * 
 */
UCLASS()
class TOYBOX_API UDashAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	UDashAbility();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	TObjectPtr<UAnimMontage> DashForwardMontage = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	TObjectPtr<UAnimMontage> DashBackwardMontage = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	TObjectPtr<UAnimMontage> DashLeftMontage = {nullptr};
	
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	TObjectPtr<UAnimMontage> DashRightMontage = {nullptr};

private:
	UFUNCTION()
	void OnRootMotionFinished();

	UFUNCTION()
	void OnDashMontageCancel();

	FVector GetFacingDirection() const;

	FVector GetAccelerationDirection() const;

	FVector GetCameraDirection() const;

	void StartDashCameraModifier() const;
	void EndDashCameraModifier() const;

	void StartDashState() const;
	void EndDashState() const;

	TObjectPtr<UAbilityTask_ApplyRootMotionConstantForce> CreateRootMotionTask(const FVector& DashDirection);
	
	TWeakObjectPtr<UAnimMontage> SelectDirectionalAnimMontage(const FVector& FacingDirection, const FVector& AccelerationDirection, FVector& DashDirection) const;

	TObjectPtr<UAbilityTask_ApplyRootMotionConstantForce> RootMotionConstantForceTask = {nullptr};

	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayDashMontageTask = {nullptr};
	
	FGameplayTag DashModiferTag = FGameplayTag::EmptyTag;
};
