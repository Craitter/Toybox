// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayCues/WeaponGameplayCue.h"
#include "Actors/ToyboxCharacter.h"
#include "Toybox/Toybox.h"

bool UWeaponGameplayCue::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UWeaponGameplayCue::OnExecute_Implementation)

	const TWeakObjectPtr<AToyboxCharacter> Character = Cast<AToyboxCharacter>(Target);

	FGameplayCueParameters NewParameters = Parameters;
	
	if (Character.IsValid())
	{
		NewParameters.TargetAttachComponent = Character->GetCurrentWeaponMesh(); 
	}
		
	return Super::OnExecute_Implementation(Target, NewParameters);
}
