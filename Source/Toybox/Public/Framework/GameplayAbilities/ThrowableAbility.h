// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "ThrowableAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class AThrowableProjectile;
/**
 * 
 */
UCLASS()
class TOYBOX_API UThrowableAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	UThrowableAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	TWeakObjectPtr<AToyboxCharacter> Character;
	
	TSubclassOf<AThrowableProjectile> ThrowableProjectile = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category="Throwable")
	TObjectPtr<UAnimMontage> Animation = {nullptr};
	
	UPROPERTY(EditDefaultsOnly, Category="Throwable")
	FName HandSocket;
	
	UFUNCTION()
	void OnMontageTaskEnds();
	
	UFUNCTION()
	void OnMontageTaskCancelled();

	void OnMeleeNotify(const FName NotifyName, IAnimNotifyInterface* AnimNotifyInterface);

private:
	bool StartAnimation();

	void SpawnProjectile();

	FVector GetProjectileSpawnLocation() const;

	FRotator GetProjectileSpawnRotation() const;
	
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = {nullptr};

	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageAndWaitTask = {nullptr};
};
