// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxPlayerState.h"
#include "LobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TOYBOX_API ALobbyPlayerState : public AToyboxPlayerState
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Server, Reliable)
	void Server_ToggleSpectator();

protected:
	virtual void CopyProperties(APlayerState* PlayerState) override;
};
