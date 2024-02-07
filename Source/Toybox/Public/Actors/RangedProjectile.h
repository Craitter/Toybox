// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "RangedProjectile.generated.h"

class ARangedWeapon;

UCLASS()
class TOYBOX_API ARangedProjectile : public AProjectile
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARangedProjectile();

	virtual void PreInitializeComponents() override;

	UFUNCTION()
	void HandleHitResult(const FHitResult& HitResult);
	
	void SetWeapon(const TObjectPtr<ARangedWeapon> NewWeapon);

	virtual void SetSpecHandle(const FGameplayEffectSpecHandle& Handle) override;

	void SetGameplayCueTag(const FGameplayTag& CueTag);
	
private:
	TObjectPtr<ARangedWeapon> Weapon = {nullptr};
	
	bool bWaitingForData = false;
	
	FHitResult CachedHitResult;

	FGameplayTag GameplayCueTag = FGameplayTag::EmptyTag;
	void DrawDecal(const FVector& Location, const FRotator& Rotation) const;
};
