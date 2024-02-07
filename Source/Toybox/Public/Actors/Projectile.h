// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class TOYBOX_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComponent = {nullptr};
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> Mesh = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = {nullptr};

	virtual void PreInitializeComponents() override;
	
	virtual void SetSpecHandle(const FGameplayEffectSpecHandle& Handle);

	virtual void PostNetReceiveLocationAndRotation() override;
	
protected:
	FGameplayEffectSpecHandle SpecHandle;

	void ApplyEffectToTarget(const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent, const FHitResult& HitResult) const;
};
