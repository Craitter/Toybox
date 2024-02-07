// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Interfaces/AnimNotifyInterface.h"
#include "ToyboxAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UToyboxAnimNotify : public UAnimNotify, public IAnimNotifyInterface
{
	GENERATED_BODY()
};
