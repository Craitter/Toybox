// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxDataTypes.h"
#include "Engine/DataAsset.h"
#include "RangedWeaponData.generated.h"

class ARangedProjectile;
/**
 * 
 */
UCLASS()
class TOYBOX_API URangedWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	ERangedWeaponOperatingType OperatingType = ERangedWeaponOperatingType::None;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> RelatedRanged = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag WallHitGameplayCueTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditDefaultsOnly, Category="BulletType")
	bool bIsProjectileBased = false;

	UPROPERTY(EditDefaultsOnly, Category="BulletType|Projectile", meta=(EditCondition = "bIsProjectileBased"))
	TSubclassOf<ARangedProjectile> Projectile = {nullptr};
	
	UPROPERTY(EditDefaultsOnly, Category="Animations")
	TObjectPtr<UAnimMontage> ReloadAnimation = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category="Animations")
	TObjectPtr<UAnimMontage> FireAnimation = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category="Decal")
	TObjectPtr<UMaterialInterface> DecalMaterial = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category="Decal")
	float DecalLifeSpan = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category="Decal")
	FVector DecalSize = FVector::ZeroVector;
};
