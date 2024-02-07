// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RangedWeapon.h"
#include "Framework/AttributeSets/RangedWeaponAttributeSet.h"
#include "HelperObjects/RangedWeaponData.h"


// Sets default values
ARangedWeapon::ARangedWeapon()
{
	RangedMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("RangedWeapon");

	if (!ensure(IsValid(RangedMeshComponent))) return;
	SetRootComponent(RangedMeshComponent);
	RangedMeshComponent->SetIsReplicated(false);

	SetReplicates(false);
}

TWeakObjectPtr<USkeletalMeshComponent> ARangedWeapon::GetRangedWeaponMesh() const
{
	return RangedMeshComponent;
}

TWeakObjectPtr<UMeshComponent> ARangedWeapon::GetWeaponMesh() const
{
	return RangedMeshComponent;
}

void ARangedWeapon::SetRangedWeaponData(const TSoftObjectPtr<URangedWeaponData>& NewData)
{
	WeaponData = NewData.LoadSynchronous();
	
	ensure(!WeaponData->RelatedRanged.IsNull());
	RangedMeshComponent->SetSkeletalMesh(WeaponData->RelatedRanged.LoadSynchronous(), false);
}

TObjectPtr<UAnimMontage> ARangedWeapon::GetReloadAnimation() const
{
	return WeaponData->ReloadAnimation;
}

ERangedWeaponOperatingType ARangedWeapon::GetOperationMode() const
{
	return WeaponData->OperatingType;
}

TWeakObjectPtr<UMaterialInterface> ARangedWeapon::GetDecalMaterial() const
{
	return WeaponData->DecalMaterial;
}

float ARangedWeapon::GetDecalLifeSpan() const
{
	return WeaponData->DecalLifeSpan;
}

FVector ARangedWeapon::GetDecalSize() const
{
	return WeaponData->DecalSize;
}

TSubclassOf<ARangedProjectile> ARangedWeapon::GetProjectile() const
{
	return WeaponData->Projectile;
}

bool ARangedWeapon::IsProjectileBased() const
{
	return WeaponData->bIsProjectileBased;
}

TObjectPtr<UAnimMontage> ARangedWeapon::GetFireAnimation() const
{
	return WeaponData->FireAnimation;
}

FGameplayTag& ARangedWeapon::GetWallHitGameplayCueTag() const
{
	return WeaponData->WallHitGameplayCueTag;
}

