// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AnimNotifyInterface.generated.h"

class AToyboxCharacter;

namespace AnimNotifyNames
{
	const FName MeleeNotify = TEXT("MeleeNotify");
	const FName MeleeComboStartNotify = TEXT("MeleeComboStart");
	const FName MeleeComboEndNotify = TEXT("MeleeComboEnd");
	const FName LetGoThrowableNotify = TEXT("LetGoThrowable");
}

USTRUCT()
struct FAnimNotifyData
{
	GENERATED_BODY()

	TWeakObjectPtr<AToyboxCharacter> OwnerCharacter = {nullptr};
	
	TArray<FHitResult> HitResultList;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAnimNotifyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOYBOX_API IAnimNotifyInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool GetAnimNotifyData(TWeakObjectPtr<USkeletalMeshComponent> OwningMesh, FAnimNotifyData& Data);
};
