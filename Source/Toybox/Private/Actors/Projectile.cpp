// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile.h"

#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Toybox/Toybox.h"


// Sets default values
AProjectile::AProjectile()
{
	bReplicates = true;
	SetReplicatingMovement(true);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("BaseSphere"));
	SetRootComponent(SphereComponent);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	Mesh->SetupAttachment(RootComponent);
	
	if (!ensure(IsValid(Mesh))) return;
	
	SphereComponent->SetCollisionProfileName(BULLET_COLLISION_PROFILE);
	SphereComponent->SetCollisionObjectType(BULLET_OBJECT_CHANNEL);

	Mesh->SetCollisionProfileName(PROFILE_NO_COLLISION);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	if (!ensure(IsValid(ProjectileMovement))) return;
	
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 5000.0f;
	ProjectileMovement->MaxSpeed = 0.0f;
	ProjectileMovement->bInterpMovement = true;
	ProjectileMovement->bInterpRotation = true;
	ProjectileMovement->SetInterpolatedComponent(Mesh);
}

void AProjectile::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	SphereComponent->IgnoreActorWhenMoving(GetOwner(), true);
}

void AProjectile::SetSpecHandle(const FGameplayEffectSpecHandle& Handle)
{
	SpecHandle = Handle;
}

void AProjectile::PostNetReceiveLocationAndRotation()
{
	ProjectileMovement->MoveInterpolationTarget(GetReplicatedMovement().Location,
		GetReplicatedMovement().Rotation);
}

void AProjectile::ApplyEffectToTarget(const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent, const FHitResult& HitResult) const
{
	if (!TargetAbilitySystemComponent.IsValid())
	{
		return;
	}
	
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	
	check(Spec != nullptr);

	Spec->GetContext().AddHitResult(HitResult, true);

	TargetAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*Spec, TargetAbilitySystemComponent.Get());
}
