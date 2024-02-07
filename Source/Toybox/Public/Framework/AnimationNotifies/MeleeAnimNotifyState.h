// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxAnimNotifyState.h"
#include "MeleeAnimNotifyState.generated.h"

class AToyboxCharacter;

DECLARE_DELEGATE_OneParam(FMeleeStateEndDelegate, TArray<TWeakObjectPtr<AActor>>&)

/**
 * 
 */
UCLASS()
class TOYBOX_API UMeleeAnimNotifyState : public UToyboxAnimNotifyState
{
	GENERATED_BODY()

public:
	UMeleeAnimNotifyState();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
							 const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual bool GetAnimNotifyData(TWeakObjectPtr<USkeletalMeshComponent> OwningMesh, FAnimNotifyData& Data) override;

protected:
	// Name of row that should be used to spawn the collision sphere
	UPROPERTY(EditAnywhere, Category="Melee")
	FName CollisionDataRowName;
	
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FAnimNotifyData> AnimNotifyDataMap;
	
	FCollisionObjectQueryParams ObjectQueryParams;
};
