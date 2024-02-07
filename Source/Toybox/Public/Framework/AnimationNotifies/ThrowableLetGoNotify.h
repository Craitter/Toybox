// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Framework/Interfaces/AnimNotifyInterface.h"
#include "ThrowableLetGoNotify.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UThrowableLetGoNotify : public UAnimNotify, public IAnimNotifyInterface
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
