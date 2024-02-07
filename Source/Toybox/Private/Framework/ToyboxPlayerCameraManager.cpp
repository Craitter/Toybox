// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ToyboxPlayerCameraManager.h"
#include "Framework/CameraModifiers/ToyboxCameraModifier.h"
#include "Toybox/Toybox.h"

TWeakObjectPtr<UCameraModifier> AToyboxPlayerCameraManager::FindCameraModifierByTag(const FGameplayTag& TagToCheck)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AToyboxPlayerCameraManager::FindCameraModifierByTag)
	
	for (UCameraModifier* Mod : ModifierList)
	{
		const TWeakObjectPtr<UToyboxCameraModifier> CameraModifier = Cast<UToyboxCameraModifier>(Mod);

		if (CameraModifier.IsValid())
		{
			if (CameraModifier->HasTag(TagToCheck))
			{
				return Mod;
			}
		}
	}
	return nullptr;
}
