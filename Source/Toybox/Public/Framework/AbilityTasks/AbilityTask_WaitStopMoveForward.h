// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitStopMoveForward.generated.h"

class UToyboxCharacterMovementComponent;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovementForwardStopped);

UCLASS()
class TOYBOX_API UAbilityTask_WaitStopMoveForward : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_WaitStopMoveForward();
	
	UPROPERTY(BlueprintAssignable)
	FOnMovementForwardStopped OnMovementForwardStopped;
	
	/** Apply force to character's movement */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_WaitStopMoveForward* WaitStopMoveForward
	(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		UMovementComponent* MovementComponent
	);
	
	virtual void TickTask(float DeltaTime) override;

private:
	void MovementStopped();
	
	TWeakObjectPtr<UToyboxCharacterMovementComponent> CharacterMovement = {nullptr};
	float MaxRadiansSprintingForward = 0.0f;
};
