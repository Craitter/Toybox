// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HelperObjects/MeleeWeaponData.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "MeleeWeapon.generated.h"

class UDataTable;
class UAnimMontage;

UCLASS()
class TOYBOX_API AMeleeWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeleeWeapon();

	virtual TWeakObjectPtr<UMeshComponent> GetWeaponMesh() const override;

	virtual TWeakObjectPtr<UStaticMeshComponent> GetMeleeWeaponMesh() const override;

	TArray<FMeleeAttack>& GetAttackList() const;

	TWeakObjectPtr<UDataTable> GetCollisionDataTable() const;

	void SetMeleeWeaponData(const TSoftObjectPtr<UMeleeWeaponData>& NewData);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> MeleeWeaponMesh = {nullptr};

	UPROPERTY()
	TObjectPtr<UMeleeWeaponData> MeleeWeaponData = {nullptr};
};
