// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/LobbyGameState.h"

#include "Actors/LobbyGameMode.h"
#include "Actors/LobbyPreviewActor.h"
#include "Actors/ToyboxPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Toybox/Toybox.h"

void FPlayerLobbyItem::SetChampion(const EChampion NewChampion)
{
	AssignedChampion = NewChampion;

	if (LobbyPreviewActor.IsValid())
	{
		LobbyPreviewActor->UpdateChampion(AssignedChampion);
	}
	if (OwningPlayerState.IsValid())
	{
		OwningPlayerState->SetSelectedChampion(AssignedChampion);
	}
}

void FPlayerLobbyItem::SetLobbyPreviewActor(const TWeakObjectPtr<ALobbyPreviewActor> NewPreviewActor)
{
	LobbyPreviewActor = NewPreviewActor;
	UpdatePreviewActor();
}

void FPlayerLobbyItem::SetIsReady(const bool bNewIsReady)
{
	bIsReady = bNewIsReady;

	if (LobbyPreviewActor.IsValid())
	{
		LobbyPreviewActor->UpdateIsReady(bIsReady);
	}
}

void FPlayerLobbyItem::ToggleIsReady()
{
	SetIsReady(!bIsReady);
}

bool FPlayerLobbyItem::IsReady() const
{
	return bIsReady;
}

void FPlayerLobbyItem::ClearLobbyPreview() const
{
	if (LobbyPreviewActor.IsValid())
	{
		LobbyPreviewActor->ClearPreview();
	}
}

void FPlayerLobbyItem::UpdatePreviewActor() const
{
	if (LobbyPreviewActor.IsValid() && OwningPlayerState.IsValid())
	{
		LobbyPreviewActor->SetPlayer(FText::FromString(OwningPlayerState->GetPlayerName()), AssignedChampion, bIsReady);
	}
}

void FPlayerLobbyItem::UpdatePlayerName() const
{
	if (LobbyPreviewActor.IsValid() && OwningPlayerState.IsValid())
	{
		LobbyPreviewActor->UpdatePlayerName(FText::FromString(OwningPlayerState->GetPlayerName()));
	}
}

void FPlayerLobbyItem::PreReplicatedRemove(const FPlayerLobbyArray& InArraySerializer) const
{
	ClearLobbyPreview();
}

void FPlayerLobbyItem::PostReplicatedAdd(const FPlayerLobbyArray& InArraySerializer) const
{
	UpdatePreviewActor();
}

void FPlayerLobbyItem::PostReplicatedChange(const FPlayerLobbyArray& InArraySerializer) const
{
	UpdatePreviewActor();
}

void FPlayerLobbyArray::AddLobbyItemForPlayer(const FPlayerLobbyItem& NewLobbyItem)
{
	if (CanAssignNewItemToPlayer(NewLobbyItem.OwningPlayerState))
	{
		FPlayerLobbyItem& Item = Items.Add_GetRef(NewLobbyItem);
		Item.UpdatePreviewActor();
		MarkItemDirty(Item);
	}
}

void FPlayerLobbyArray::RemoveLobbyItemForPlayer(const TWeakObjectPtr<APlayerState> PlayerState)
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		const FPlayerLobbyItem& Item = Items[i];
		if (Item.OwningPlayerState == PlayerState)
		{
			Item.ClearLobbyPreview();
			Items.RemoveAt(i);
			MarkArrayDirty();
			break;
		}
	}
}

void FPlayerLobbyArray::UpdatePlayerName(const TWeakObjectPtr<APlayerState> PlayerState)
{
	for (FPlayerLobbyItem& Item : Items)
	{
		if (Item.OwningPlayerState == PlayerState)
		{
			Item.UpdatePlayerName();
			MarkItemDirty(Item);
			return;
		}
	}
}

void FPlayerLobbyArray::TogglePlayerReady(const TWeakObjectPtr<APlayerState> PlayerState)
{
	for (FPlayerLobbyItem& Item : Items)
	{
		if (Item.OwningPlayerState == PlayerState)
		{
			Item.ToggleIsReady();
			MarkItemDirty(Item);
			break;
		}
	}
}

bool FPlayerLobbyArray::ArePlayersReady()
{
	for (const FPlayerLobbyItem& Item : Items)
	{
		if (!Item.IsReady())
		{
			return false;
		}
	}
	return true;
}

TArray<EToyline> FPlayerLobbyArray::GetAssignedToylines() const
{
	TArray<EToyline> AssignedToylines;
	for (const FPlayerLobbyItem& Item : Items)
	{
		AssignedToylines.Add(Item.AssignedToyline);
	}
	return AssignedToylines;
}

bool FPlayerLobbyArray::IsPlayerRegistered(TWeakObjectPtr<APlayerState> PlayerState)
{
	for (const FPlayerLobbyItem& Item : Items)
	{
		if (Item.OwningPlayerState == PlayerState)
		{
			return true;
		}
	}
	return false;
}

bool FPlayerLobbyArray::CanAssignNewItemToPlayer(const TWeakObjectPtr<APlayerState> PlayerState)
{
	for (const FPlayerLobbyItem& Item : Items)
	{
		if (Item.OwningPlayerState == PlayerState)
		{
			return false;
		}
	}
	return true;
}

void ALobbyGameState::RegisterPreviewActor(const TWeakObjectPtr<ALobbyPreviewActor> LobbyPreviewActor)
{
	LobbyPreviewActors.Add(LobbyPreviewActor);
}

void ALobbyGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	check(GetWorld())
	
	if (HasAuthority())
	{
		for (const TWeakObjectPtr<APlayerState> PlayerState : PlayerArray)
		{
			if (!TryMakePlayerActive(PlayerState))
			{
				MakePlayerSpectator(PlayerState);
			}
		}
	}
}

void ALobbyGameState::HandleMatchHasEnded()
{
	check(GetWorld())
	
	if (AuthorityLobbyGameMode.IsValid())
	{
		GetWorld()->ServerTravel(FString::Printf(TEXT("%s?listen"), *AuthorityLobbyGameMode->GetDefaultMatchMap().GetAssetName()));
	}
}

void ALobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (HasAuthority() && bReplicatedHasBegunPlay)
	{
		if (!TryMakePlayerActive(PlayerState))
		{
			MakePlayerSpectator(PlayerState);
		}
	}
}

void ALobbyGameState::CreateLobbyDataForPlayer(const TWeakObjectPtr<APlayerState> PlayerState)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ALobbyGameState::CreateLobbyDataForPlayer)

	FPlayerLobbyItem NewLobbyItem = FPlayerLobbyItem();
	NewLobbyItem.OwningPlayerState = Cast<AToyboxPlayerState>(PlayerState);
	NewLobbyItem.AssignedToyline = GetRandomUnassignedToyline();
	NewLobbyItem.SetChampion(GetNextChampionForToyline(NewLobbyItem.AssignedToyline, EChampion::None));
	NewLobbyItem.SetLobbyPreviewActor(GetUnassignedPreviewActor());
	NewLobbyItem.SetIsReady(false);
	
	PlayerLobbyArray.AddLobbyItemForPlayer(NewLobbyItem);
	NotifyActivePlayerCountChanged();
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	
	if (HasAuthority())
	{
		RemoveLobbyDataForPlayer(PlayerState);
	}
}

bool ALobbyGameState::IsPlayerSpectator(const TWeakObjectPtr<APlayerState> PlayerState)
{
	return !PlayerLobbyArray.IsPlayerRegistered(PlayerState);
}

int32 ALobbyGameState::GetCurrentActivePlayerCount() const
{
	return PlayerLobbyArray.Num();
}

void ALobbyGameState::RemoveLobbyDataForPlayer(const TWeakObjectPtr<APlayerState> PlayerState)
{
	PlayerLobbyArray.RemoveLobbyItemForPlayer(PlayerState);
	NotifyActivePlayerCountChanged();
	UpdateCanStartMatch();
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerLobbyArray);
}

void ALobbyGameState::ReceivedGameModeClass()
{
	Super::ReceivedGameModeClass();
	
	AuthorityLobbyGameMode = Cast<ALobbyGameMode>(AuthorityGameMode);
}

void ALobbyGameState::UpdatePlayerName(const TObjectPtr<APlayerState> PlayerState, const FString& NewName)
{
	PlayerLobbyArray.UpdatePlayerName(PlayerState);
}

EToyline ALobbyGameState::GetRandomUnassignedToyline() const
{
	if (!AuthorityLobbyGameMode.IsValid())
	{
		return EToyline::None;
	}
	TArray<EToyline> PossibleToylines;
	AuthorityLobbyGameMode->GetUnassignedToylines(PossibleToylines, GetAssignedToylines());
	
	const int32 RandomIndex = FMath::RandHelper(PossibleToylines.Num() - 1);
	return PossibleToylines.IsValidIndex(RandomIndex) ? PossibleToylines[RandomIndex] : EToyline::None;
}

TArray<EToyline> ALobbyGameState::GetAssignedToylines() const
{
	return PlayerLobbyArray.GetAssignedToylines();
}

EChampion ALobbyGameState::GetNextChampionForToyline(const EToyline Toyline, const EChampion CurrentChampion) const
{
	if (!AuthorityLobbyGameMode.IsValid())
	{
		return EChampion::None;
	}
	
	return AuthorityLobbyGameMode->GetNextChampionForToyline(Toyline, CurrentChampion);
}

TWeakObjectPtr<ALobbyPreviewActor> ALobbyGameState::GetUnassignedPreviewActor()
{
	for (TWeakObjectPtr<ALobbyPreviewActor> PreviewActor : LobbyPreviewActors)
	{
		if (PreviewActor.IsValid() && !PreviewActor->IsAssigned())
		{
			return PreviewActor;
		}
	}
	return nullptr;
}

void ALobbyGameState::MakePlayerSpectator(const TWeakObjectPtr<APlayerState> PlayerState)
{
	RemoveLobbyDataForPlayer(PlayerState);
	PlayerState->SetIsSpectator(true);
	PlayerState->SetIsOnlyASpectator(true);
}

bool ALobbyGameState::TryMakePlayerActive(const TWeakObjectPtr<APlayerState> PlayerState)
{
	if (PlayerLobbyArray.Num() < DESIRED_PLAYER_COUNT)
	{
		CreateLobbyDataForPlayer(PlayerState);
		PlayerState->SetIsSpectator(false);
		PlayerState->SetIsOnlyASpectator(false);
		return true;
	}
	return false;
}

void ALobbyGameState::TogglePlayerIsReady(APlayerState* OwningPlayerState)
{
	ToggleReadyForPlayer(OwningPlayerState);
}

void ALobbyGameState::ToggleReadyForPlayer(const TWeakObjectPtr<APlayerState> PlayerState)
{
	PlayerLobbyArray.TogglePlayerReady(PlayerState);
	UpdateCanStartMatch();
}

bool ALobbyGameState::CanStartMatch()
{
	return PlayerLobbyArray.Num() == DESIRED_PLAYER_COUNT && PlayerLobbyArray.ArePlayersReady();
}

void ALobbyGameState::UpdateCanStartMatch()
{
	const bool bNewCanStartMatch = CanStartMatch();
	
	if (GetCurrentCanStartMatch() != bNewCanStartMatch)
	{
		SetCanStartMatch(bNewCanStartMatch);
	}
}

void ALobbyGameState::SetCanStartMatch(const bool bNewCanStartMatch)
{
	bOldCanStartMatch = bNewCanStartMatch;
	NotifyCanStartMatchChanged();
}

bool ALobbyGameState::GetCurrentCanStartMatch() const
{
	return bOldCanStartMatch;
}

void ALobbyGameState::NotifyCanStartMatchChanged() const
{
	if (!MatchCanStartDelegate.ExecuteIfBound(GetCurrentCanStartMatch()))
	{
		UE_LOG(LogTemp, Error, TEXT("No one listens to MatchCanStartDelegate so we dont need to, %s %s()"),
			   *UEnum::GetValueAsString(GetLocalRole()), *FString(__FUNCTION__));
	}
}

void ALobbyGameState::NotifyActivePlayerCountChanged() const
{
	OnActivePlayerCountChangedDelegate.Broadcast(PlayerLobbyArray.Num());
}

void ALobbyGameState::OnRep_PlayerLobbyArray() const
{
	NotifyActivePlayerCountChanged();
}

