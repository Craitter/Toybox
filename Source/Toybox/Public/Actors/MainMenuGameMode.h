// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxGameMode.h"
#include "MainMenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API AMainMenuGameMode : public AToyboxGameMode
{
	GENERATED_BODY()

public:
	TSoftObjectPtr<UWorld> GetLobbyMap(); 
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> LobbyMap = {nullptr};
};
