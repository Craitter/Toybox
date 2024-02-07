// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayCues/DamageTakenCue.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Actors/ToyboxCharacter.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


ADamageTakenCue::ADamageTakenCue()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	GameplayCueTag = NativeTags.GameplayCue_Character_DamageTaken;
}

bool ADamageTakenCue::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ADamageTakenCue::OnExecute_Implementation)

	const TWeakObjectPtr<AToyboxCharacter> Character = Cast<AToyboxCharacter>(MyTarget);
	if (Character.IsValid() && Character->GetCurrentMontage() == nullptr)
	{
		const bool bIsAlive = Character->IsAlive();

		if (!ensure(Parameters.EffectContext.GetHitResult())) return false;
		
		const EHitDirection Direction = GetHitDirection(Parameters.EffectContext.GetHitResult()->ImpactPoint, MyTarget);

		const TWeakObjectPtr<UAnimMontage> HitReactToPlay = GetAnimationToPlay(bIsAlive, Direction);
		
		Character->PlayAnimMontage(HitReactToPlay.Get());
	}
	
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}

TWeakObjectPtr<UAnimMontage> ADamageTakenCue::GetAnimationToPlay(const bool bIsAlive,
	const EHitDirection Direction) const
{
	switch (Direction)
	{
	case Front:
		return GetRandomAnimationFromList(FrontHitReacts);
	case Left:
		return GetRandomAnimationFromList(LeftHitReacts);
	case Right:
		return GetRandomAnimationFromList(RightHitReacts);
	case Back:
		return GetRandomAnimationFromList(BackHitReacts);
	default:
		return nullptr;
	}
}

EHitDirection ADamageTakenCue::GetHitDirection(const FVector& ImpactPoint, const TWeakObjectPtr<AActor> HitActor)
{
	const FVector& ActorLocation = HitActor->GetActorLocation();

	const float DistanceToFrontBackPlane = FVector::PointPlaneDist(ImpactPoint, ActorLocation, HitActor->GetActorRightVector());
	const float DistanceToRightLeftPlane = FVector::PointPlaneDist(ImpactPoint, ActorLocation, HitActor->GetActorForwardVector());
	
	if (FMath::Abs(DistanceToFrontBackPlane) <= FMath::Abs(DistanceToRightLeftPlane))
	{
		if (DistanceToRightLeftPlane >= 0)
		{
			return Front;
		}
		
		return Back;
	}
	
	if (DistanceToFrontBackPlane >= 0)
	{
		return Right;
	}
	
	return Left;
}

TWeakObjectPtr<UAnimMontage> ADamageTakenCue::GetRandomAnimationFromList(const TArray<TObjectPtr<UAnimMontage>>& MontageList)
{
	if (MontageList.IsEmpty()) return nullptr;
	return MontageList[FMath::RandRange(0, MontageList.Num() - 1)];
}
