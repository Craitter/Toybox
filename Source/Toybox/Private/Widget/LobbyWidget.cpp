// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/LobbyWidget.h"

#include "Actors/LobbyGameState.h"
#include "Actors/LobbyPlayerState.h"
#include "Actors/ToyboxPlayerState.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"
#include "Toybox/Toybox.h"

static constexpr float GSend_Ready_Delay = 0.2f;

void ULobbyWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	
	const TWeakObjectPtr<APlayerController> OwningPlayer = GetOwningPlayer();
	GameState = GetWorld()->GetGameState<ALobbyGameState>();
	
	if (!OwningPlayer.IsValid()) return;
	if (!GameState.IsValid()) return;
	if (!IsValid(ReadyButton)) return;
	if (!IsValid(StartGameButton)) return;
	if (!IsValid(ToggleSpectatorButton)) return;
	if (!IsValid(ToggleSpectatorText)) return;
	
	ReadyButton->OnClicked.AddDynamic(this, &ThisClass::OnReadyButtonPressed);
	
	if (!OwningPlayer->HasAuthority())
	{
		ReadyButton->SetVisibility(ESlateVisibility::Hidden);
		ToggleSpectatorButton->SetVisibility(ESlateVisibility::Hidden);
		
		ToggleSpectatorButton->OnClicked.AddDynamic(this, &ThisClass::OnToggleSpectatorButtonPressed);
		GameState->OnActivePlayerCountChangedDelegate.AddUObject(this, &ThisClass::OnActivePlayerCountChanged);
		OnActivePlayerCountChanged(GameState->GetCurrentActivePlayerCount());
		
		StartGameButton->SetVisibility(ESlateVisibility::Collapsed);		
	}
	else
	{
		ToggleSpectatorButton->SetVisibility(ESlateVisibility::Collapsed);
		
		StartGameButton->OnClicked.AddDynamic(this, &ThisClass::OnStartGameButtonPressed);
		StartGameButton->SetIsEnabled(false);
			
		GameState->MatchCanStartDelegate.BindUObject(this, &ThisClass::OnMatchCanStartUpdated);
	}	
	
}

void ULobbyWidget::OnReadyButtonPressed()
{
	//Avoid network killing with spamming (consider moving to 0.1s instead so it doesnt feel unresponsive)
	check(GetWorld());
	if (ReadyNetworkSaferTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ReadyNetworkSaferTimerHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(ReadyNetworkSaferTimerHandle, this, &ThisClass::SendToggleReady, GSend_Ready_Delay);
}

void ULobbyWidget::OnStartGameButtonPressed()
{
	if (GameState.IsValid())
	{
		GameState->SetMatchState(MatchState::WaitingPostMatch);
	}
}

void ULobbyWidget::OnToggleSpectatorButtonPressed()
{
	const TWeakObjectPtr<ALobbyPlayerState> PlayerState = GetOwningPlayerState<ALobbyPlayerState>();
	if (PlayerState.IsValid())
	{
		ToggleSpectatorButton->SetIsEnabled(false);
		PlayerState->Server_ToggleSpectator();
	}
}

void ULobbyWidget::OnActivePlayerCountChanged(const int32 PlayerCount) const
{
	if (GameState->IsPlayerSpectator(GetOwningPlayerState()))
	{
		ShowSpectatorOverlay();
		ToggleSpectatorButton->SetIsEnabled(PlayerCount < DESIRED_PLAYER_COUNT);
		ToggleSpectatorText->SetText(FText::FromString(TEXT("Leave Spectators")));
	}
	else
	{
		ShowActivePlayerOverlay();
		ToggleSpectatorButton->SetIsEnabled(true);
		ToggleSpectatorText->SetText(FText::FromString(TEXT("Join Spectators")));
	}
	ToggleSpectatorButton->SetVisibility(ESlateVisibility::Visible);
}

void ULobbyWidget::OnMatchCanStartUpdated(const bool bCanStart) const
{
	if (!IsValid(StartGameButton))
	{
		return;
	}
	
	if (bCanStart)
	{
		StartGameButton->SetIsEnabled(true);
	}
	else
	{
		StartGameButton->SetIsEnabled(false);
	}
}

void ULobbyWidget::SendToggleReady() const
{
	const TWeakObjectPtr<AToyboxPlayerState> PlayerState = GetOwningPlayerState<AToyboxPlayerState>();
	if (PlayerState.IsValid())
	{
		PlayerState->Server_TogglePlayerIsReady();
	}
}

void ULobbyWidget::ShowActivePlayerOverlay() const
{
	ReadyButton->SetVisibility(ESlateVisibility::Visible);
}

void ULobbyWidget::ShowSpectatorOverlay() const
{
	ReadyButton->SetVisibility(ESlateVisibility::Hidden);
}
