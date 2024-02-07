// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/LobbyGameMode.h"

#include "Actors/LobbyGameState.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "Toybox/Toybox.h"


ALobbyGameMode::ALobbyGameMode()
{
	bUseSeamlessTravel = true;
}

EChampion ALobbyGameMode::GetNextChampionForToyline(const EToyline Toyline, const EChampion CurrentChampion)
{
	TArray<EChampion>& Data = CachedToylineSpecificChampionData.FindOrAdd(Toyline);
	if (Data.IsEmpty())
	{
		GetAllChampionDataForToyline(Toyline, Data);
	}
	int32 NextChampionIndex = 0;
	if (CurrentChampion != EChampion::None)
	{
		if (Data.Find(CurrentChampion, NextChampionIndex))
		{
			NextChampionIndex++;
		}
	}
	if (NextChampionIndex >= Data.Num())
	{
		NextChampionIndex = 0;
	}
	return Data.IsValidIndex(NextChampionIndex) ? Data[NextChampionIndex] : EChampion::None;
}

bool ALobbyGameMode::IsToylineInUse(const EToyline Toyline, const TArray<EToyline>& InUsedToylines)
{
	return InUsedToylines.Find(Toyline) != INDEX_NONE;
}

void ALobbyGameMode::ChangeName(AController* Controller, const FString& NewName, bool bNameChange)
{
	Super::ChangeName(Controller, NewName, bNameChange);

	const TWeakObjectPtr<ALobbyGameState> LobbyGameState = Cast<ALobbyGameState>(GameState);
	if (LobbyGameState.IsValid() && IsValid(Controller))
	{
		LobbyGameState->UpdatePlayerName(Controller->PlayerState, NewName);
	}
}

TSoftObjectPtr<UWorld> ALobbyGameMode::GetDefaultMatchMap()
{
	return DefaultMatchMap;
}

void ALobbyGameMode::GetUnassignedToylines(TArray<EToyline>& OutUnusedToylines, const TArray<EToyline>& InUsedToylines)
{
	for (const EToyline& Toyline : TEnumRange<EToyline>())
	{
		if (IsToylineInUse(Toyline, InUsedToylines))
		{
			continue;
		}
		
		OutUnusedToylines.Add(Toyline);
	}
}

void ALobbyGameMode::GetAllChampionDataForToyline(const EToyline Toyline, TArray<EChampion>& OutData) const
{
	if (!IsValid(ToylineDataTable))
	{
		return;		
	}
	TArray<FChampionData*> AllChampions;
	ToylineDataTable->GetAllRows(FString(TEXT("Trying to get all rows in GameMode")), AllChampions);
	for (const FChampionData* Champion : AllChampions)
	{
		if (Champion != nullptr && Champion->Toyline == Toyline)
		{
			OutData.AddUnique(Champion->Champion);
		}
	}
}
