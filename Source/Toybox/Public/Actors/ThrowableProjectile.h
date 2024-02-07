// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ThrowableProjectile.generated.h"

class UGameplayEffect;

UCLASS()
class TOYBOX_API AThrowableProjectile : public AProjectile
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AThrowableProjectile();
	
	virtual void LifeSpanExpired() override;
	
	void SetSourceAbilitySystem(TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent);
	
	float GetCooldown() const;
	
	UPROPERTY(EditDefaultsOnly)
	float CollisionRadius = 1000.0f;

	UPROPERTY(EditDefaultsOnly)
	bool bEnableDebug = false;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> ThrowableEffect = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag EffectCallerTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly)
	float Magnitude = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float Cooldown = 10.0f;
	
private:
	void GetValidHitResults(TArray<FHitResult>& HitResults) const;

	void DoSphereCollisionSweep(TArray<FHitResult>& HitResults) const;

	bool CanDamageActor(TWeakObjectPtr<UPrimitiveComponent> Component) const;

#if ENABLE_DRAW_DEBUG
	void DebugTrace(const FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const;
#endif
};
