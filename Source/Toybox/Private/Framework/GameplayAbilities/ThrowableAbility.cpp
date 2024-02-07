// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/ThrowableAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Actors/ThrowableProjectile.h"
#include "Framework/Interfaces/AnimNotifyInterface.h"
#include "Toybox/Toybox.h"


UThrowableAbility::UThrowableAbility()
{
}

void UThrowableAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UThrowableAbility::ActivateAbility)

	if (ActorInfo->AbilitySystemComponent == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	
	AbilitySystemComponent = ActorInfo->AbilitySystemComponent;
	
	Character = Cast<AToyboxCharacter>(ActorInfo->AvatarActor.Get());

	if (!Character.IsValid())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	Character->OnAnimNotifyEvent.AddUObject(this, &ThisClass::OnMeleeNotify);

	ThrowableProjectile = Character->GetThrowableProjectile();
	
	if (!ensure(IsValid(ThrowableProjectile)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (!StartAnimation())
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}

	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UThrowableAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	// if (IsValid(PlayMontageAndWaitTask))
	// {
	// 	PlayMontageAndWaitTask->ExternalCancel();
	// }
	//
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UThrowableAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (Character.IsValid() && Character->OnAnimNotifyEvent.IsBoundToObject(this))
	{
		Character->OnAnimNotifyEvent.RemoveAll(this);
	}
}

void UThrowableAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void UThrowableAbility::OnMontageTaskEnds()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UThrowableAbility::OnMontageTaskCancelled()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UThrowableAbility::OnMeleeNotify(const FName NotifyName, IAnimNotifyInterface* AnimNotifyInterface)
{
	if (NotifyName == AnimNotifyNames::LetGoThrowableNotify)
	{
		SpawnProjectile();
	}
}

bool UThrowableAbility::StartAnimation()
{
	PlayMontageAndWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("ThrowableAttackMontageTask"), Animation);
		
	if (!IsValid(PlayMontageAndWaitTask))
	{
		return false;
	}

	PlayMontageAndWaitTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageTaskCancelled);
	PlayMontageAndWaitTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageTaskCancelled);
	PlayMontageAndWaitTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageTaskEnds);

	PlayMontageAndWaitTask->ReadyForActivation();

	return true;
}

void UThrowableAbility::SpawnProjectile()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UThrowableAbility::SpawnProjectile)
	
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}
	
	const FVector ProjectileSpawn = GetProjectileSpawnLocation();

	const FRotator Rotation = GetProjectileSpawnRotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character.Get();

	const TWeakObjectPtr<AThrowableProjectile> Projectile = GetWorld()->SpawnActor<AThrowableProjectile>(
		ThrowableProjectile, ProjectileSpawn, Rotation, SpawnParams);

	if (Projectile.IsValid())
	{
		Projectile->SetSourceAbilitySystem(AbilitySystemComponent);

		CooldownDuration = Projectile->GetCooldown();

		ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	}
}

FVector UThrowableAbility::GetProjectileSpawnLocation() const
{
	const TWeakObjectPtr<USkeletalMeshComponent> SkeletalMesh = Character->GetMesh();

	if (!ensure(SkeletalMesh->DoesSocketExist(HandSocket)))
	{
		return FVector::ZeroVector;
	}
	
	return SkeletalMesh->GetSocketLocation(HandSocket);
}

FRotator UThrowableAbility::GetProjectileSpawnRotation() const
{
	// Ignoring this but need to have one 
	FVector CameraLocation;
	
	FRotator Rotation;

	Character->GetController()->GetPlayerViewPoint(CameraLocation, Rotation);

	return Rotation;
}

