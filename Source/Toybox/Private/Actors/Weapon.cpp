// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Weapon.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetActorEnableCollision(false);

	bReplicates = false;
}

TWeakObjectPtr<UMeshComponent> AWeapon::GetWeaponMesh() const
{
	return nullptr;
}

TWeakObjectPtr<USkeletalMeshComponent> AWeapon::GetRangedWeaponMesh() const
{
	return nullptr;
}

TWeakObjectPtr<UStaticMeshComponent> AWeapon::GetMeleeWeaponMesh() const
{
	return nullptr;
}

