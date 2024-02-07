// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "RangedAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class URecoilCameraModifier;
class UOffsetCameraModifier;
class UAbilityTask_WaitInputRelease;
class UWeaponAttributeSet;
class UReloadAbility;
/**
 * 
 */
UCLASS()
class TOYBOX_API URangedAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	URangedAbility();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void OnPawnAvatarSet() override;

	UFUNCTION()
	void OnAbilityInputRelease(float TimeWaited);

	
	
	UPROPERTY(EditDefaultsOnly)
	FName BulletOriginSocket = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, Category="Attacks")
	TSubclassOf<UGameplayEffect> DealDamageGameplayEffect = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category="Attacks")
	TSubclassOf<UGameplayEffect> BulletSpreadGameplayEffect = {nullptr};
	
private:
	void FireRangedWeapon();

	void RequestReloadRanged() const;

	void TraceShot(FHitResult& HitResult) const;

	void DoSingleLineTrace(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const;

	void DoMultiLineTrace(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const;

	void DoCameraLineTrace(FHitResult& HitResult) const;
	
	void GetCameraTraceVectors(FVector& TraceStart, FVector& TraceEnd) const;
	
	void GetGunTraceVectors(FVector& TraceStart, FVector& TraceEnd, const FHitResult& HitResult = FHitResult()) const;
	
	void HandleHitResult(const FHitResult& HitResult) const;
	
	void DealDamage(const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent, const FHitResult& HitResult) const;

	void DebugTrace(const FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const;

	void DoContinuousFire();

	void DoSingleFire();

	void DoBurstFire();

	bool CanBurst() const;

	bool CanStartBurstAbility() const;

	void EndAbilityOnInputReleased();

	void FireTraceShot() const;

	void FireProjectileShot() const;

	void SpawnProjectile() const;

	void FireShot() const;

	void ApplyRecoil() const;

	void DrawDecal(const FVector& Location, const FRotator& Rotation) const;

	void UpdateRangedAttributeSet();

	void ApplyBulletSpread(FRotator& ShotRotation) const;

	void UpdateCurrentBulletSpread(const float BaseSpread, const float MinAccuracy, const float MaxSpread) const;
	
	FRotator GetBulletSpreadRotator(float Spread) const;

	FGameplayEffectSpecHandle GetDamageSpecHandle() const;
	
	FVector GetProjectileTarget() const;
	
	TWeakObjectPtr<URecoilCameraModifier> GetRecoilCameraModifier(TWeakObjectPtr<APlayerController> Controller) const;

	float GetDotProductToLocation(const FVector& TargetLocation) const;

	FVector GetHitResultLocation(const FHitResult& HitResult) const;

	bool TrySetFireAnimation();

	void GetTraceQueryParams(FCollisionQueryParams& TraceQueryParams) const;
	
	FGameplayTag RecoilModifierTag = FGameplayTag::EmptyTag;
	
	TWeakObjectPtr<ARangedWeapon> Weapon = {nullptr};

	TWeakObjectPtr<const UWeaponAttributeSet> RangedAttributeSet = {nullptr};

	FGameplayTag ReloadAbilityTag;

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = {nullptr};

	TWeakObjectPtr<AToyboxCharacter> Character = {nullptr};

	FGameplayTag DamageModiferTag = FGameplayTag::EmptyTag;
	
	FGameplayTag BulletSpreadModiferTag = FGameplayTag::EmptyTag;

	TObjectPtr<UAbilityTask_WaitInputRelease> AbilityInputReleasedTask = {nullptr};

	TObjectPtr<UAbilityTask_PlayMontageAndWait> FireAnimationTask = {nullptr};

	FTimerHandle BurstShotTimer;
	
	// Used to keep track of what shot we're on in a burst
	int32 CurrentShotInBurst = 0;

	TWeakObjectPtr<URecoilCameraModifier> RecoilModifer;
	
	FVector GunSocketVector = FVector::ZeroVector;
};
