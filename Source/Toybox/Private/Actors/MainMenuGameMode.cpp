// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MainMenuGameMode.h"

TSoftObjectPtr<UWorld> AMainMenuGameMode::GetLobbyMap()
{
	return LobbyMap;
}
