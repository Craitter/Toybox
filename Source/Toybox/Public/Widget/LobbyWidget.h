// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

class ALobbyGameState;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class TOYBOX_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartGameButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ToggleSpectatorButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ToggleSpectatorText = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyText = {nullptr};

private:
	UFUNCTION()
	void OnReadyButtonPressed();

	UFUNCTION()
	void OnStartGameButtonPressed();

	UFUNCTION()
	void OnToggleSpectatorButtonPressed();

	void OnActivePlayerCountChanged(const int32 PlayerCount) const;

	void OnMatchCanStartUpdated(bool bCanStart) const;

	TWeakObjectPtr<ALobbyGameState> GameState = {nullptr};

	FTimerHandle ReadyNetworkSaferTimerHandle;

	void SendToggleReady() const;

	void ShowActivePlayerOverlay() const;

	void ShowSpectatorOverlay() const;
};
