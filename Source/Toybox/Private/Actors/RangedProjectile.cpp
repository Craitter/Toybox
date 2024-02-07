// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RangedProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "Actors/RangedWeapon.h"
#include "Components/DecalComponent.h"
#include "Engine/DecalActor.h"
#include "Framework/ToyboxAbilitySystemComponent.h"
#include "Framework/GameplayAbilities/RangedAbility.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Toybox/Toybox.h"

// Sets default values
ARangedProjectile::ARangedProjectile()
{
}

void ARangedProjectile::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ThisClass::HandleHitResult);
}

void ARangedProjectile::DrawDecal(const FVector& Location, const FRotator& Rotation) const 
{
	FActorSpawnParameters SpawnParameters;

	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// You have to change the relative rotation instead of world rotation for the decal to look right (No idea why!)
	const TWeakObjectPtr<ADecalActor> Decal = GetWorld()->SpawnActor<ADecalActor>(Location,
		FRotator::ZeroRotator, SpawnParameters);
	if (Decal.IsValid())
	{
		Decal->SetActorEnableCollision(false);
		Decal->GetDecal()->FadeScreenSize = DECAL_FADE_SCREEN_SIZE;
		Decal->SetDecalMaterial(Weapon->GetDecalMaterial().Get());
		Decal->SetLifeSpan(Weapon->GetDecalLifeSpan());
		Decal->GetDecal()->DecalSize = Weapon->GetDecalSize();

		Decal->SetActorRelativeRotation(Rotation);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Tried to spawn in decal but failed too! %s"), *FString(__FUNCTION__));
	}
}

void ARangedProjectile::HandleHitResult(const FHitResult& HitResult)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ARangedProjectile::HandleHitResult)

	if (!IsValid(Weapon) || !SpecHandle.IsValid())
	{
		CachedHitResult = HitResult;
		bWaitingForData = true;
		SetLifeSpan(.3f);
		return;
	}
	
	const TWeakObjectPtr<AActor> HitActor = HitResult.GetActor();
	if (!HitActor.IsValid())
	{
		Destroy();
		return;
	}

	const IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(HitActor);

	if (AbilityInterface != nullptr)
	{
		ApplyEffectToTarget(AbilityInterface->GetAbilitySystemComponent(), HitResult);
	}
	else
	{
		if (GameplayCueTag != FGameplayTag::EmptyTag)
		{
			UToyboxAbilitySystemComponent::CreateGameplayCueFromHitResult(GetOwner(), GameplayCueTag, HitResult);
		}
		
		DrawDecal(HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
	}

	Destroy();
}

void ARangedProjectile::SetWeapon(const TObjectPtr<ARangedWeapon> NewWeapon)
{
	Weapon = NewWeapon;

	if (bWaitingForData)
	{
		HandleHitResult(CachedHitResult);
	}
}

void ARangedProjectile::SetSpecHandle(const FGameplayEffectSpecHandle& Handle)
{
	Super::SetSpecHandle(Handle);

	if (bWaitingForData)
	{
		HandleHitResult(CachedHitResult);
	}
}

void ARangedProjectile::SetGameplayCueTag(const FGameplayTag& CueTag)
{
	GameplayCueTag = CueTag;
}
