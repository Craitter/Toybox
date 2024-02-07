// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxGameMode.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "LobbyGameMode.generated.h"

struct FChampionData;
enum class EToyline;
class UDataTable;
/**
 * 
 */

UCLASS()
class TOYBOX_API ALobbyGameMode : public AToyboxGameMode
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	EChampion GetNextChampionForToyline(const EToyline Toyline, const EChampion CurrentChampion);

	static void GetUnassignedToylines(TArray<EToyline>& OutUnusedToylines, const TArray<EToyline>& InUsedToylines);
	static bool IsToylineInUse(const EToyline Toyline, const TArray<EToyline>& InUsedToylines);

	virtual void ChangeName(AController* Controller, const FString& NewName, bool bNameChange) override;

	TSoftObjectPtr<UWorld> GetDefaultMatchMap();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	TObjectPtr<UDataTable> ToylineDataTable = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DefaultMatchMap = {nullptr};
	
	void GetAllChampionDataForToyline(const EToyline Toyline, TArray<EChampion>& OutData) const;

private:
	TMap<EToyline, TArray<EChampion>> CachedToylineSpecificChampionData;

};
