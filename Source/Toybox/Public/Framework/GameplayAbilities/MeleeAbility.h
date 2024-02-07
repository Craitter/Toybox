// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "MeleeAbility.generated.h"

struct FMeleeAttack;
class UAbilityTask_WaitInputPress;
class UAbilityTask_WaitGameplayTagAdded;
class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class TOYBOX_API UMeleeAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	UMeleeAbility();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void OnMontageTaskEnds();
	
	UFUNCTION()
	void OnMontageTaskCancelled();	

	UFUNCTION()
	void OnComboFailed();

	UFUNCTION()
	void OnAbilityInputPressed(float TimeWaited);

	void OnMeleeNotify(const FName NotifyName, IAnimNotifyInterface* AnimNotifyInterface);

	// The time it takes for you to no longer be able to start the next combo after the animation is complete
	UPROPERTY(EditDefaultsOnly, Category="Attacks|Combo")
	float ComboFailDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category="Attacks")
	TSubclassOf<UGameplayEffect> DealDamageGameplayEffect = {nullptr};

private:
	void SetCanCombo(bool bNewValue);
	
	bool StartAttack();

	void MeleeHit(const TArray<FHitResult>& HitResults);

	void DealDamage(const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent, const FHitResult& HitResult) const;
	
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageAndWaitTask = {nullptr};
	
	TObjectPtr<UAbilityTask_WaitInputPress> AbilityInputPressedTask = {nullptr};

	FDelegateHandle AnimNotifyEventHandle;

	FTimerHandle ComboFailDelayTimer;
	
	int32 CurrentAttackIndex = 0;
	
	bool bCanCombo = false;

	TArray<TWeakObjectPtr<AActor>> AlreadyHitActors;

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = {nullptr};
	
	FGameplayTag DamageModiferTag = FGameplayTag::EmptyTag;
	
	TArray<FMeleeAttack> ListOfAttacks;

	TWeakObjectPtr<AToyboxCharacter> Character = {nullptr};
};
