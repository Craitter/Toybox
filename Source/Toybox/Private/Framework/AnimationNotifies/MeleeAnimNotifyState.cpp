// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AnimationNotifies/MeleeAnimNotifyState.h"
#include "Actors/MeleeWeapon.h"
#include "Actors/ToyboxCharacter.h"
#include "Toybox/Toybox.h"


FString GDataTableContextString = TEXT("UMeleeAnimNotifyState");

UMeleeAnimNotifyState::UMeleeAnimNotifyState()
{
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif 
}

void UMeleeAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	// If the map already contains the meshcomp we have nothing else todo!
	if (AnimNotifyDataMap.Contains(MeshComp))
	{
		return;
	}

	const TWeakObjectPtr<AToyboxCharacter> OwnerCharacter = Cast<AToyboxCharacter>(MeshComp->GetOwner());
	if (!OwnerCharacter.IsValid())
	{
		return;
	}

	FAnimNotifyData CharacterData;
	CharacterData.OwnerCharacter = OwnerCharacter;
	
	// Set the owner character so we can use it in all notifies
	AnimNotifyDataMap.Add(MeshComp, CharacterData);
}

void UMeleeAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!AnimNotifyDataMap.Contains(MeshComp))
	{
		Super::NotifyEnd(MeshComp, Animation, EventReference);
		return;
	}

	FAnimNotifyData& NotifyData = AnimNotifyDataMap[MeshComp];
	
	if (!NotifyData.OwnerCharacter.IsValid())
	{
		Super::NotifyEnd(MeshComp, Animation, EventReference);
		return;
	}

	if (MeshComp->GetOwnerRole() == ROLE_Authority)
	{
		NotifyData.OwnerCharacter->OnAnimNotifyEvent.Broadcast(AnimNotifyNames::MeleeNotify, this);
	}
	NotifyData.HitResultList.Reset();
	
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

void UMeleeAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UMeleeAnimNotifyState::NotifyTick)

	FAnimNotifyData* NotifyData = AnimNotifyDataMap.Find(MeshComp);

	
	checkf(NotifyData != nullptr, TEXT("Notify of MeshComp is ticking but mesh isnt in AnimNotifyDataMap"))
	
	if (!NotifyData->OwnerCharacter.IsValid())
	{
		return;
	}

	const TWeakObjectPtr<AMeleeWeapon> MeleeWeapon = NotifyData->OwnerCharacter->GetEquippedMelee();

	if (!MeleeWeapon.IsValid())
	{
		return;
	}

	const TWeakObjectPtr<UDataTable> WeaponData = MeleeWeapon->GetCollisionDataTable();

	if (!WeaponData.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("There is no data table assigned to the weapon: %s"), *MeleeWeapon->GetName())
		return;
	}

	const FMeleeCollisionData* CollisionData = WeaponData->FindRow<FMeleeCollisionData>(CollisionDataRowName,
		GDataTableContextString);
	
	if (CollisionData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("There collision data row name is invalid for supplied data table. Rowname: %s"), *CollisionDataRowName.ToString())
		return;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(NotifyData->OwnerCharacter.Get());

	const FRotator OwnerRotator = NotifyData->OwnerCharacter->GetActorRotation();
	const FVector RotatedVector = OwnerRotator.RotateVector(CollisionData->RelativeOffset);
	const FVector CollisionSpawnLocation = NotifyData->OwnerCharacter->GetActorLocation() + RotatedVector;

	const float Radius = CollisionData->Radius;
	
	// Perform the multi-sphere overlap check
	TArray<FHitResult> HitResults;
	const bool bBlockingHit = MeleeWeapon->GetWorld()->SweepMultiByObjectType(
		HitResults,
		CollisionSpawnLocation,
		CollisionSpawnLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	if (bBlockingHit)
	{
		for (const FHitResult& HitResult : HitResults)
		{
			// This is checked later on so for performance we use Add instead of AddUnique
			NotifyData->HitResultList.Add(HitResult);
		}
	}

#if ENABLE_DRAW_DEBUG
	if (NotifyData->OwnerCharacter->bEnableWeaponDebug)
	{
		DrawDebugSphere(MeleeWeapon->GetWorld(), CollisionSpawnLocation, Radius, 32,
			bBlockingHit ? FColor::Green : FColor::Red, false, FrameDeltaTime + 2.0f,
			0.0f, 0.5f);
	}
#endif
	
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}

bool UMeleeAnimNotifyState::GetAnimNotifyData(TWeakObjectPtr<USkeletalMeshComponent> OwningMesh, FAnimNotifyData& Data)
{
	if (!AnimNotifyDataMap.Contains(OwningMesh))
	{
		return false;
	}
	
	Data = AnimNotifyDataMap[OwningMesh];
	return true;
}
