// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ToyboxPlayerStart.h"


AToyboxPlayerStart::AToyboxPlayerStart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ToylineDefenderSpawnMesh = CreateDefaultSubobject<UStaticMeshComponent>("DefenderMesh");

	if (!ensure(IsValid(ToylineDefenderSpawnMesh))) return;

	ToylineDefenderSpawnMesh->SetupAttachment(RootComponent);

	bReplicates = true;
	ToylineDefenderSpawnMesh->SetIsReplicated(true);
}

void AToyboxPlayerStart::SetMesh(const TWeakObjectPtr<UStaticMesh> DefenderSpawnMesh)
{
	ToylineDefenderSpawnMesh->SetStaticMesh(DefenderSpawnMesh.Get());
	bIsInitialized = true;
}

bool AToyboxPlayerStart::IsPlayerStartInitialized() const
{
	return bIsInitialized;
}
