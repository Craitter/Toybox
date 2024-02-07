// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "MatchSetup.generated.h"

class UToyboxAbilitySet;
class APlayerStart;
class AToyboxPlayerController;
class AMatchPlayerState;
class ULevelStreamingDynamic;
class AArenaActor;

USTRUCT()
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AToyboxPlayerController> PlayerController = {nullptr};
		
	UPROPERTY()
	TWeakObjectPtr<AArenaActor> ArenaActor = {nullptr};

	UPROPERTY()
	TSoftObjectPtr<UWorld> WorldToStream = {nullptr};
	
	UPROPERTY(NotReplicated)
	TWeakObjectPtr<ULevelStreamingDynamic> LoadedTemplate = {nullptr};
	
	UPROPERTY(NotReplicated)
	TArray<TWeakObjectPtr<APlayerStart>> TemplateAvailablePlayerStarts;

	UPROPERTY(NotReplicated)
	bool bWasDefender = false;

	bool HasUniqueIdChanged() const;
	void SetUniqueId(int32 UniqueId);
	int32 GetUniqueId() const;
	void UpdateLastId();

	bool operator==(const FPlayerData& Other) const
	{
		if (LastUniqueID == Other.LastUniqueID)
		{
			return true;
		}
		return false;
	}

private:
	UPROPERTY()
	int32 UniqueLevelStreamingIndex = INDEX_NONE;
	
	UPROPERTY(NotReplicated)
	int32 LastUniqueID = INDEX_NONE;
};


UCLASS()
class TOYBOX_API AMatchSetup : public AInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMatchSetup(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool bCanChangeTemplate = true;

	TWeakObjectPtr<ULevel> GetLevelOfPlayer(TWeakObjectPtr<APlayerController> Controller); 

protected:
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_MatchData)
	TArray<FPlayerData> MatchData;
	
	TArray<TSoftObjectPtr<UWorld>> Templates;

	TArray<TWeakObjectPtr<AArenaActor>> ArenaArray;

public:
	void AddPlayer(TWeakObjectPtr<APlayerController> PlayerController);

	void SetAvailableTemplates(const TArray<TSoftObjectPtr<UWorld>>& AvailableTemplates);

	//StartSpot
protected:
	void CacheStartSpotsForAllPlayers();

	static void CacheStartSpotsForPlayer(FPlayerData& PlayerData);

	static void GetPlayerStartsForStreamedLevel(const TWeakObjectPtr<ULevelStreamingDynamic> StreamedLevel, TArray<TWeakObjectPtr<APlayerStart>>& PlayerStarts);
	
	void SetCurrentAttackerStartSpots(const TArray<TObjectPtr<APlayerStart>>& AvailablePlayerStarts);

	TArray<TObjectPtr<APlayerStart>> CurrentAttackerStartSpots;
	
public:
	TArray<TObjectPtr<APlayerStart>> GetCurrentAttackerStartSpots() const;

	TWeakObjectPtr<APlayerStart> GetDefenderStartSpot(TWeakObjectPtr<APlayerController> PlayerController);
	//StartSpot end
	
public:
	void ProcessBattleStart();

	void ProcessBattleEnd();
	
	void ProcessPreparationPhaseStart();

	bool WasEveryPlayerDefender();
	
	void CacheArenaActors();

	TWeakObjectPtr<AArenaActor> GetAssignedArenaActor(TWeakObjectPtr<APlayerController> Controller);
	
	UFUNCTION()
	void OnRep_MatchData();
	
	TWeakObjectPtr<ULevelStreamingDynamic> StreamDynamicLevel(const FPlayerData& Data);

	void LoadNextTemplate(TWeakObjectPtr<APlayerController> PlayerController);

	void LoadPreviousTemplate(TWeakObjectPtr<APlayerController> PlayerController);

	void AssignTeamsByDefender(TWeakObjectPtr<APlayerController> DefenderPlayerController);

	void RemoveAllTeams();
protected:
	void AssignPlayer(FPlayerData& Data);


	FPlayerData* GetMatchDataForPlayer(TWeakObjectPtr<APlayerController> PlayerController);

	int32 GetCurrentTemplateIndex(const TSoftObjectPtr<UWorld>& CurrentStreamedWorld) const;

	void UpdateWorldToStream(FPlayerData& Data, const int32 NewIndex);

	FPlayerData* GetNewDefender();
	
};