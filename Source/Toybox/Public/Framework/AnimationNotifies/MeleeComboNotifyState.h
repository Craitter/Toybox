// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxAnimNotifyState.h"
#include "MeleeComboNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UMeleeComboNotifyState : public UToyboxAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
							 const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
