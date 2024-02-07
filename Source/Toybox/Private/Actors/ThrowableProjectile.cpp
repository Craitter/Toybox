// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ThrowableProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "Toybox/Toybox.h"


// Sets default values
AThrowableProjectile::AThrowableProjectile()
{
	// Set a starting lifespan so the the grenade will always be deleted eventually
	InitialLifeSpan = 30.0f;
}

void AThrowableProjectile::LifeSpanExpired()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AThrowableProjectile::LifeSpanExpired)

	TArray<FHitResult> HitResults;
	GetValidHitResults(HitResults);

	for (const FHitResult& HitResult : HitResults)
	{
		TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitResult.GetActor(), false);

		if (AbilitySystemComponent.IsValid())
		{
			ApplyEffectToTarget(AbilitySystemComponent, HitResult);
		}
	}
	
	Super::LifeSpanExpired();
}

void AThrowableProjectile::SetSourceAbilitySystem(TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent)
{
	const FGameplayEffectSpecHandle NewSpecHandle = AbilitySystemComponent->
		MakeOutgoingSpec(ThrowableEffect, 1.0f, AbilitySystemComponent->MakeEffectContext());

	FGameplayEffectSpec* Spec = NewSpecHandle.Data.Get();

	if (Spec != nullptr && EffectCallerTag != FGameplayTag::EmptyTag)
	{
		Spec->SetSetByCallerMagnitude(EffectCallerTag, Magnitude);
	}
	
	SetSpecHandle(NewSpecHandle);
}

float AThrowableProjectile::GetCooldown() const
{
	return Cooldown;
}


void AThrowableProjectile::GetValidHitResults(
	TArray<FHitResult>& HitResults) const
{
	TArray<FHitResult> RawHitResults;
	DoSphereCollisionSweep(RawHitResults);

	TArray<TWeakObjectPtr<AActor>> AlreadyAddedActors;
	for (const FHitResult& HitResult : RawHitResults)
	{
		const TWeakObjectPtr<AActor> HitActor = HitResult.GetActor();
			
		if (!HitActor.IsValid() && AlreadyAddedActors.Find(HitActor) != INDEX_NONE)
		{
			continue;
		}

		const IAbilitySystemInterface* AbilityInterface = Cast<IAbilitySystemInterface>(HitActor);
		
		if (AbilityInterface != nullptr && CanDamageActor(HitResult.GetComponent()))
		{
			HitResults.Add(HitResult);
			AlreadyAddedActors.Add(HitActor);
		}
	}
}

void AThrowableProjectile::DoSphereCollisionSweep(TArray<FHitResult>& HitResults) const
{
	const FVector CollisionSpawnLocation = GetActorLocation();

	const FCollisionQueryParams QueryParams;
	GetWorld()->SweepMultiByProfile(
		HitResults,
		CollisionSpawnLocation,
		CollisionSpawnLocation,
		FQuat::Identity,
		PROFILE_OVERLAP_ALL,
		FCollisionShape::MakeSphere(CollisionRadius),
		QueryParams);

#if ENABLE_DRAW_DEBUG
	if (bEnableDebug)
	{
		DrawDebugSphere(GetWorld(), CollisionSpawnLocation, CollisionRadius, 32,
			FColor::Red, false, 2.0f, 0.0f, 1.0f);
	}
#endif
}

bool AThrowableProjectile::CanDamageActor(TWeakObjectPtr<UPrimitiveComponent> Component) const
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Component->GetOwner());
	
	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByProfile(HitResults, GetActorLocation(),
		Component->GetOwner()->GetActorLocation(), PROFILE_OVERLAP_ALL, QueryParams);

	for (FHitResult& HitResult : HitResults)
	{
		TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitResult.GetActor(), false);

		if (!AbilitySystemComponent.IsValid())
		{
#if ENABLE_DRAW_DEBUG
			HitResult.bBlockingHit = true;
			DebugTrace(HitResult, GetActorLocation(), HitResult.GetActor()->GetActorLocation());
#endif
	
			return false;
		}
	}

#if ENABLE_DRAW_DEBUG
	FHitResult DebugHit;
	DebugHit.bBlockingHit = false;
	
	DebugTrace(DebugHit, GetActorLocation(), Component->GetOwner()->GetActorLocation());
#endif
	
	return true;
}

#if ENABLE_DRAW_DEBUG
void AThrowableProjectile::DebugTrace(const FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const
{
	if (bEnableDebug)
	{
		FColor DebugColor = FColor::Green;
		FVector DebugTraceStart = TraceStart;

		if (HitResult.bBlockingHit)
		{
			DrawDebugLine(
				GetWorld(),
				TraceStart,
				HitResult.ImpactPoint,
				DebugColor,
				false, 2.0f, 0,
				1.0f
			);

			DebugTraceStart = HitResult.ImpactPoint;
			DebugColor = FColor::Red;
		}

		DrawDebugLine(
			GetWorld(),
			DebugTraceStart,
			TraceEnd,
			DebugColor,
			false, 2.0f, 0,
			1.0f
		);
	}
}
#endif