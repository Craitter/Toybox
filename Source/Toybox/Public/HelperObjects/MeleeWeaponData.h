// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxDataTypes.h"
#include "Engine/DataAsset.h"
#include "MeleeWeaponData.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API UMeleeWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> RelatedMelee = {nullptr};
	
	UPROPERTY(EditDefaultsOnly)
	TArray<FMeleeAttack> AttackList;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> CollisionDataTable = {nullptr};
};
