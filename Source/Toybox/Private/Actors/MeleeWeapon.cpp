// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MeleeWeapon.h"
#include "Engine/DataTable.h"

// Sets default values
AMeleeWeapon::AMeleeWeapon()
{
	MeleeWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>("MeleeWeapon");

	if (!ensure(IsValid(MeleeWeaponMesh))) return;
	SetRootComponent(MeleeWeaponMesh);
	MeleeWeaponMesh->SetIsReplicated(false);
}

TWeakObjectPtr<UMeshComponent> AMeleeWeapon::GetWeaponMesh() const
{
	return MeleeWeaponMesh;
}

TWeakObjectPtr<UStaticMeshComponent> AMeleeWeapon::GetMeleeWeaponMesh() const
{
	return MeleeWeaponMesh;
}

TArray<FMeleeAttack>& AMeleeWeapon::GetAttackList() const
{
	return MeleeWeaponData->AttackList;
}

TWeakObjectPtr<UDataTable> AMeleeWeapon::GetCollisionDataTable() const
{
	return MeleeWeaponData->CollisionDataTable;
}

void AMeleeWeapon::SetMeleeWeaponData(const TSoftObjectPtr<UMeleeWeaponData>& NewData)
{
	MeleeWeaponData = NewData.LoadSynchronous();
	MeleeWeaponMesh->SetStaticMesh(MeleeWeaponData->RelatedMelee.LoadSynchronous());
}

void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();
}
