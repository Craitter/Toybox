// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ToyboxPlayerState.h"

#include "Actors/ToyboxGameState.h"
#include "Net/UnrealNetwork.h"

AToyboxPlayerState::AToyboxPlayerState()
{
}

void AToyboxPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, SelectedChampion);
}

void AToyboxPlayerState::Server_TogglePlayerIsReady_Implementation()
{
	check(GetWorld())
	
	if (GameState.IsValid())
	{
		GameState->TogglePlayerIsReady(this);
	}
}

void AToyboxPlayerState::BeginPlay()
{
	Super::BeginPlay();
	GameState = Cast<AToyboxGameState>(GetWorld()->GetGameState());
}

void AToyboxPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
}

void AToyboxPlayerState::SetSelectedChampion(const EChampion NewChampion)
{
	SelectedChampion = NewChampion;
	OnRep_SelectedChampion();
}
