// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxGameState.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LobbyGameState.generated.h"

/*	For more Information see ADR in drive and Flowcharts
 *
 * This is a description of the Lobby data flow, how every client knows of each other and when which functions will be called
 * The Central point of the Replication and Data sync is the Struct FPlayerLobbyArray
 * 1. HandleMatchHasStarted will iterate through all PlayerStates that have been added prior to World->BeginPlay()
 * it will Call CreateLobbyItemForPlayer(), which assigns a Toyline a Champion and a PreviewActor to a PlayerState,
 * this data is hold by the Struct FPlayerLobbyItem and passed to FPlayerLobbyArray
 *
 * 2. Every new Player that joins after BeginPlay() happened on the Server will be caught @AddNewPlayerState() and will
 * call CreateLobbyItemForPlayer() as well
 * 
 * 3. After the Creation of a FPlayerLobbyItem it will be passed over to FPlayerLobbyArray::AddLobbyItemForPlayer, this
 * struct updates the Preview Actor to display the data depending on the assigned champion, player and if the player is
 * ready, it will also mark the property Dirty so it gets then replicated to all clients
 *
 * The Property FPlayerLobbyArray::Items is protected to avoid forgetting to mark things as dirty and allows a
 * controlled and centralized data flow
 *
 * 4. Clients Receive the Replicated Data, Thanks to the FFastArraySerializer.h we have access to functions
 * that get called when a replicated change / add / remove happens, so whenever a item updates we can update the
 * preview actor accordingly, this leads to a very safe data flow with little to no overhead,
 * because we dont have to update the whole array each time we update one variable
 *
 * 5. The Server didnt receive the ClientsPlayerNames when AddNewPlayerState is being called...
 * The GameMode notifies the GameState when a Player changes its name, see @ALobbyGameMode::ChangeName,
 * which is called everytime a player changes its name (and then updates the playerstates)
 * Eventhough clients dont have this problem, we want to make it consistent and mark the property dirty afterwards,
 * in case this data wasnt ready the moment the FPlayerLobbyArray replicated down and it will ensure that the
 * playername would also be up to date if the player changes his name
 *
 * 6. When a Player presses ready it will be sent to the Server GameState.
 * the GameState will then check if all players are ready UpdateCanStartMatch() and if we are enough players, if we are,
 * the host can start the Match by updating the current Match state to WaitingPostMatch
 * HandleMatchHasEnded() will start a seamless server travel
 *
 * Everytime the Champion of a Player gets updated the Playerstate will be synced (on the server)
 * so we can rely on the data being there anytime
 *
 * 7. if a player leaves it will be caught in RemovePlayerState() and it will remove the matching
 * FPlayerLobbyArray Item and then UpdateCanStartMatch()
*/

class AToyboxPlayerState;
class ALobbyPlayerState;
class ALobbyGameMode;
class ALobbyPreviewActor;

DECLARE_DELEGATE_OneParam(FMatchCanStartDelegate, bool /*bCanBeStarted*/)

DECLARE_MULTICAST_DELEGATE_OneParam(FOnActivePlayerCountChangedDelegate, int32 /*NewCount*/)

USTRUCT()
struct FPlayerLobbyItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AToyboxPlayerState> OwningPlayerState = {nullptr};
	
	UPROPERTY(NotReplicated)
	EToyline AssignedToyline = EToyline::None;

	void SetChampion(EChampion NewChampion);
	
	void SetLobbyPreviewActor(TWeakObjectPtr<ALobbyPreviewActor> NewPreviewActor);

	void SetIsReady(bool bNewIsReady);

	void ToggleIsReady();

	bool IsReady() const;

	void ClearLobbyPreview() const;

	void UpdatePreviewActor() const;

	void UpdatePlayerName() const;
	
protected:
	UPROPERTY()
	EChampion AssignedChampion = EChampion::None;
	
	UPROPERTY()
	TWeakObjectPtr<ALobbyPreviewActor> LobbyPreviewActor = {nullptr};

	UPROPERTY()
	bool bIsReady = false;

public:
	/** 
	 * Optional functions you can implement for client side notification of changes to items; 
	 * Parameter type can match the type passed as the 2nd template parameter in associated call to FastArrayDeltaSerialize
	 * 
	 * NOTE: It is not safe to modify the contents of the array serializer within these functions, nor to rely on the contents of the array 
	 * being entirely up-to-date as these functions are called on items individually as they are updated, and so may be called in the middle of a mass update.
	 */
	void PreReplicatedRemove(const struct FPlayerLobbyArray& InArraySerializer) const;
	void PostReplicatedAdd(const struct FPlayerLobbyArray& InArraySerializer) const;
	void PostReplicatedChange(const struct FPlayerLobbyArray& InArraySerializer) const;
};

USTRUCT()
struct FPlayerLobbyArray : public FFastArraySerializer
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	TArray<FPlayerLobbyItem>	Items;

public:	
	void AddLobbyItemForPlayer(const FPlayerLobbyItem& NewLobbyItem);
	
	void RemoveLobbyItemForPlayer(TWeakObjectPtr<APlayerState> PlayerState);

	void UpdatePlayerName(const TWeakObjectPtr<APlayerState> PlayerState);

	void TogglePlayerReady(const TWeakObjectPtr<APlayerState> PlayerState);
	
	bool ArePlayersReady();
	
	TArray<EToyline> GetAssignedToylines() const;

	bool IsPlayerRegistered(TWeakObjectPtr<APlayerState> PlayerState);

protected:
	bool CanAssignNewItemToPlayer(const TWeakObjectPtr<APlayerState> PlayerState);
	
public:
	FORCEINLINE int32 Num() const
	{
		return Items.Num();
	}
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FPlayerLobbyItem, FPlayerLobbyArray>(Items, DeltaParms, *this);
	}
};

template <>
struct TStructOpsTypeTraits<FPlayerLobbyArray> : public TStructOpsTypeTraitsBase2<FPlayerLobbyArray>
{
	enum 
	{
		WithNetDeltaSerializer = true,
	};
};

/**
 * 
 */
UCLASS()
class TOYBOX_API ALobbyGameState : public AToyboxGameState
{
	GENERATED_BODY()

public:
	//Begin override AGameState
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;

	virtual void ReceivedGameModeClass() override;

	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	//End override AGameState

	TWeakObjectPtr<ALobbyGameMode> AuthorityLobbyGameMode = {nullptr};

	void UpdatePlayerName(TObjectPtr<APlayerState> PlayerState, const FString& NewName);
	
protected:
	virtual void HandleMatchHasStarted() override;

	virtual void HandleMatchHasEnded() override;
	
public:
	FOnActivePlayerCountChangedDelegate OnActivePlayerCountChangedDelegate;

	bool IsPlayerSpectator(TWeakObjectPtr<APlayerState> PlayerState);

	int32 GetCurrentActivePlayerCount() const;
private:
	void RemoveLobbyDataForPlayer(TWeakObjectPtr<APlayerState> PlayerState);

	void CreateLobbyDataForPlayer(TWeakObjectPtr<APlayerState> PlayerState);
	
protected:
	EToyline GetRandomUnassignedToyline() const;

public:
	TArray<EToyline> GetAssignedToylines() const;

protected:
	EChampion GetNextChampionForToyline(const EToyline Toyline, const EChampion CurrentChampion) const;

public:
	void RegisterPreviewActor(TWeakObjectPtr<ALobbyPreviewActor> LobbyPreviewActor);

protected:	
	//returns null when there is no UnsetPreviewActor
	TWeakObjectPtr<ALobbyPreviewActor> GetUnassignedPreviewActor();

private:
	TArray<TWeakObjectPtr<ALobbyPreviewActor>> LobbyPreviewActors;

public:
	void MakePlayerSpectator(TWeakObjectPtr<APlayerState> PlayerState);
	
	bool TryMakePlayerActive(TWeakObjectPtr<APlayerState> PlayerState);
	
	virtual void TogglePlayerIsReady(APlayerState* OwningPlayerState) override;
	
	FMatchCanStartDelegate MatchCanStartDelegate;
	
private:
	void ToggleReadyForPlayer(TWeakObjectPtr<APlayerState> PlayerState);
	bool CanStartMatch();

	void UpdateCanStartMatch();

	void SetCanStartMatch(bool bNewCanStartMatch);

	bool GetCurrentCanStartMatch() const;

	void NotifyCanStartMatchChanged() const;
	
	bool bOldCanStartMatch = false;

	void NotifyActivePlayerCountChanged() const;
	
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_PlayerLobbyArray)
	FPlayerLobbyArray PlayerLobbyArray;

	UFUNCTION()
	void OnRep_PlayerLobbyArray() const;
};
