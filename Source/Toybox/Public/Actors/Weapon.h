// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	virtual TWeakObjectPtr<USkeletalMeshComponent> GetRangedWeaponMesh() const;

	virtual TWeakObjectPtr<UStaticMeshComponent> GetMeleeWeaponMesh() const;

	virtual TWeakObjectPtr<UMeshComponent> GetWeaponMesh() const;
};
