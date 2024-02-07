// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "RangedWeapon.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;
class UWeaponAttributeSet;

UCLASS()
class TOYBOX_API ARangedWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARangedWeapon();

	virtual TWeakObjectPtr<USkeletalMeshComponent> GetRangedWeaponMesh() const override;

	virtual TWeakObjectPtr<UMeshComponent> GetWeaponMesh() const override;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> RangedMeshComponent = {nullptr};
	
	// Setters
	void SetRangedWeaponData(const TSoftObjectPtr<URangedWeaponData>& NewData);
	
	// Getters
	TObjectPtr<UAnimMontage> GetReloadAnimation() const;

	ERangedWeaponOperatingType GetOperationMode() const;

	TWeakObjectPtr<UMaterialInterface> GetDecalMaterial() const;

	float GetDecalLifeSpan() const;

	FVector GetDecalSize() const;

	TSubclassOf<ARangedProjectile> GetProjectile() const;
	
	bool IsProjectileBased() const;

	TObjectPtr<UAnimMontage> GetFireAnimation() const;

	FGameplayTag& GetWallHitGameplayCueTag() const;
	
private:
	UPROPERTY()
	TObjectPtr<URangedWeaponData> WeaponData;
};
