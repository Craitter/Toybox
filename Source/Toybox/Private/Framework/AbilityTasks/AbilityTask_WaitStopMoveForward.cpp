// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AbilityTasks/AbilityTask_WaitStopMoveForward.h"
#include "Framework/ToyboxCharacterMovementComponent.h"

UAbilityTask_WaitStopMoveForward::UAbilityTask_WaitStopMoveForward()
{
	bTickingTask = true;
}

UAbilityTask_WaitStopMoveForward* UAbilityTask_WaitStopMoveForward::WaitStopMoveForward(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, UMovementComponent* MovementComponent)
{
	const TWeakObjectPtr<UToyboxCharacterMovementComponent> ToyboxMovement = Cast<UToyboxCharacterMovementComponent>(MovementComponent);
	if (!ensure(ToyboxMovement.IsValid()))
	{
		return nullptr;
	}
	
	UAbilityTask_WaitStopMoveForward* MyTask = NewAbilityTask<UAbilityTask_WaitStopMoveForward>(OwningAbility, TaskInstanceName);

	MyTask->CharacterMovement = ToyboxMovement;
	MyTask->MaxRadiansSprintingForward = FMath::DegreesToRadians(ToyboxMovement->MaxDegreeSprintingForward);

	return MyTask;
}

void UAbilityTask_WaitStopMoveForward::TickTask(float DeltaTime)
{
	if (!CharacterMovement.IsValid())
	{
		MovementStopped();
		return;
	}

	const FVector AccelerationNormal = CharacterMovement->GetCurrentAcceleration().GetSafeNormal2D();
	if (AccelerationNormal.IsNearlyZero() || !IsValid(CharacterMovement->GetOwner()))
	{
		MovementStopped();
		return;
	}

	const FVector& ForwardVector = CharacterMovement->GetOwner()->GetActorForwardVector();
	
	if (acos(AccelerationNormal.Dot(ForwardVector)) > MaxRadiansSprintingForward)
	{
		MovementStopped();
		return;
	}
}

void UAbilityTask_WaitStopMoveForward::MovementStopped()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnMovementForwardStopped.Broadcast();
	}
	EndTask();
}
