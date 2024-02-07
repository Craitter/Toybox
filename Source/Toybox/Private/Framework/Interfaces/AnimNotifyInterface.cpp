// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Interfaces/AnimNotifyInterface.h"

// Add default functionality here for any IAnimNotifyInterface functions that are not pure virtual.

bool IAnimNotifyInterface::GetAnimNotifyData(TWeakObjectPtr<USkeletalMeshComponent> OwningMesh, FAnimNotifyData& Data)
{
	return false;
}
