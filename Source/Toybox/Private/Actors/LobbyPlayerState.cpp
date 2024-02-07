// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/LobbyPlayerState.h"

#include "Actors/LobbyGameState.h"

void ALobbyPlayerState::Server_ToggleSpectator_Implementation()
{
	check(GetWorld())

	const TWeakObjectPtr<ALobbyGameState> LobbyGameState = Cast<ALobbyGameState>(GameState);
	if (LobbyGameState.IsValid())
	{
		if (LobbyGameState->IsPlayerSpectator(this))
		{
			LobbyGameState->TryMakePlayerActive(this);
		}
		else
		{
			LobbyGameState->MakePlayerSpectator(this);
		}
	}
}

void ALobbyPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	const TWeakObjectPtr<AToyboxPlayerState> NewPlayerState = Cast<AToyboxPlayerState>(PlayerState);
	if (NewPlayerState.IsValid())
	{
		NewPlayerState->SetSelectedChampion(SelectedChampion);
		NewPlayerState->SetIsOnlyASpectator(IsSpectator());
	}
}
