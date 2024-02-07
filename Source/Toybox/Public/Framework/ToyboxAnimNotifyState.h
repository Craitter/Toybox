// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Interfaces/AnimNotifyInterface.h"
#include "ToyboxAnimNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UToyboxAnimNotifyState : public UAnimNotifyState, public IAnimNotifyInterface
{
	GENERATED_BODY()
};
