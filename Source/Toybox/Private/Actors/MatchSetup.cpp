// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MatchSetup.h"

#include "EngineUtils.h"
#include "Actors/ArenaActor.h"
#include "Actors/MatchPlayerState.h"
#include "Actors/ToyboxPlayerController.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/PlayerStart.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Toybox/Toybox.h"

bool FPlayerData::HasUniqueIdChanged() const
{
	return LastUniqueID != UniqueLevelStreamingIndex;
}

void FPlayerData::SetUniqueId(int32 UniqueId)
{
	UniqueLevelStreamingIndex = UniqueId;
	UpdateLastId();
}

int32 FPlayerData::GetUniqueId() const
{
	return UniqueLevelStreamingIndex;
}

void FPlayerData::UpdateLastId()
{
	LastUniqueID = UniqueLevelStreamingIndex;
}

AMatchSetup::AMatchSetup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AMatchSetup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	

	DOREPLIFETIME(ThisClass, MatchData);
}

TWeakObjectPtr<ULevel> AMatchSetup::GetLevelOfPlayer(const TWeakObjectPtr<APlayerController> Controller)
{
	for (FPlayerData& Data : MatchData)
	{
		if (Controller == Data.PlayerController)
		{
			return Data.LoadedTemplate.IsValid() ? Data.LoadedTemplate->GetLoadedLevel() : nullptr;
		}
	}
	return nullptr;
}

void AMatchSetup::CacheStartSpotsForAllPlayers()
{
	for (FPlayerData& Data : MatchData)
	{
		CacheStartSpotsForPlayer(Data);
	}
}

void AMatchSetup::CacheStartSpotsForPlayer(FPlayerData& PlayerData)
{
	if (PlayerData.TemplateAvailablePlayerStarts.IsEmpty())
	{
		GetPlayerStartsForStreamedLevel(PlayerData.LoadedTemplate, PlayerData.TemplateAvailablePlayerStarts);
	}
}

void AMatchSetup::GetPlayerStartsForStreamedLevel(const TWeakObjectPtr<ULevelStreamingDynamic> StreamedLevel,
												  TArray<TWeakObjectPtr<APlayerStart>>& PlayerStarts)
{
	PlayerStarts.Reset();
	const TWeakObjectPtr<ULevel> Level = StreamedLevel->GetLoadedLevel();
	if (Level.IsValid())
	{
		for (const TWeakObjectPtr<AActor> Actor : Level->Actors)
		{
			if (Actor->IsA(APlayerStart::StaticClass()))
			{
				PlayerStarts.Add(Cast<APlayerStart>(Actor));
			}
		}
	}
}

void AMatchSetup::SetCurrentAttackerStartSpots(
	const TArray<TObjectPtr<APlayerStart>>& AvailablePlayerStarts)
{
	CurrentAttackerStartSpots = AvailablePlayerStarts;
}

TWeakObjectPtr<APlayerStart> AMatchSetup::GetDefenderStartSpot(
	const TWeakObjectPtr<APlayerController> PlayerController)
{
	FPlayerData* PlayerData = GetMatchDataForPlayer(PlayerController);
	
	if (PlayerData == nullptr)
	{
		return nullptr;
	}
	
	CacheStartSpotsForPlayer(*PlayerData);
		
	for (const TWeakObjectPtr<APlayerStart> PlayerStart : PlayerData->TemplateAvailablePlayerStarts)
	{
		if (PlayerStart->PlayerStartTag == PLAYER_START_DEFENDER)
		{
			return PlayerStart;
		}
	}
	return nullptr;
}

void AMatchSetup::AddPlayer(const TWeakObjectPtr<APlayerController> PlayerController)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMatchSetup::AddPlayer)
	
	FPlayerData Data;
	Data.PlayerController = Cast<AToyboxPlayerController>(PlayerController);
	
	AssignPlayer(Data);
	
	MatchData.Add(Data);
}

void AMatchSetup::ProcessPreparationPhaseStart()
{
	bCanChangeTemplate = false;
	CacheStartSpotsForAllPlayers();
}

bool AMatchSetup::WasEveryPlayerDefender()
{
	for (const FPlayerData& Data : MatchData)
	{
		if (!Data.bWasDefender)
		{
			return false;
		}
	}
	return true;
}

void AMatchSetup::AssignPlayer(FPlayerData& Data)
{
	for (TWeakObjectPtr<AArenaActor> Arena : ArenaArray)
	{
		if (!Arena.IsValid() || Arena->IsAssigned())
		{
			continue;
		}
		
		Data.ArenaActor = Arena;
		Data.ArenaActor->SetIsAssigned(true);
		Data.WorldToStream = Data.ArenaActor->GetDefaultTemplate();
		Data.SetUniqueId(GetUniqueLevelStreamingId());
		Data.LoadedTemplate = StreamDynamicLevel(Data);
		break;
	}
}

void AMatchSetup::AssignTeamsByDefender(TWeakObjectPtr<APlayerController> DefenderPlayerController)
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	for (const FPlayerData& Data : MatchData)
	{
		check(Data.PlayerController.IsValid())

		TWeakObjectPtr<AMatchPlayerState> PlayerState = Data.PlayerController->GetPlayerState<AMatchPlayerState>();
		check(PlayerState.Get());
		
		if (Data.PlayerController == DefenderPlayerController)
		{
			PlayerState->SetTeam(NativeTags.Team_Defender);
			UE_LOG(LogTemp, Log, TEXT("%s is now Defender"), *Data.PlayerController->GetName());
		}
		else
		{
			PlayerState->SetTeam(NativeTags.Team_Attacker);
			UE_LOG(LogTemp, Log, TEXT("%s is now Attacker"), *Data.PlayerController->GetName());
		}
	}
}

void AMatchSetup::RemoveAllTeams()
{
	TArray<FPlayerData*> PlayersForRemoval;
	for (FPlayerData& Data : MatchData)
	{
		if (!Data.PlayerController.IsValid())
		{
			PlayersForRemoval.Add(&Data);
			continue;
		}

		TWeakObjectPtr<AMatchPlayerState> PlayerState = Data.PlayerController->GetPlayerState<AMatchPlayerState>();
		check(PlayerState.Get());
		PlayerState->SetTeam(FGameplayTag::EmptyTag);
	}
	
	for (const FPlayerData* Data : PlayersForRemoval)
	{
		MatchData.Remove(*Data);
	}
}

FPlayerData* AMatchSetup::GetMatchDataForPlayer(const TWeakObjectPtr<APlayerController> PlayerController)
{
	for (FPlayerData& Data : MatchData)
	{
		if (Data.PlayerController == PlayerController)
		{
			return &Data;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Couldnt find Match Data for Player %s"), *PlayerController->GetName());
	return nullptr;
}

int32 AMatchSetup::GetCurrentTemplateIndex(const TSoftObjectPtr<UWorld>& CurrentStreamedWorld) const
{
	return Templates.Find(CurrentStreamedWorld);
}

void AMatchSetup::UpdateWorldToStream(FPlayerData& Data, const int32 NewIndex)
{
	if (!Templates.IsValidIndex(NewIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldnt Update the Stream world because the passed Index was Invalid"));
		return;
	}
	if (!bCanChangeTemplate) return;
	
	Data.WorldToStream = Templates[NewIndex];
	Data.SetUniqueId(GetUniqueLevelStreamingId());
	Data.LoadedTemplate = StreamDynamicLevel(Data);
}

FPlayerData* AMatchSetup::GetNewDefender()
{
	TArray<FPlayerData*> AvailableDefenders;
	
	for (FPlayerData& Data : MatchData)
	{
		if (!Data.bWasDefender)
		{
			AvailableDefenders.Add(&Data);
		}
	}
	if (AvailableDefenders.IsEmpty())
	{
		return nullptr;
	}
	
	const int32 RandomIndex = FMath::RandRange(0, AvailableDefenders.Num() - 1);
	return AvailableDefenders[RandomIndex];
}

void AMatchSetup::SetAvailableTemplates(const TArray<TSoftObjectPtr<UWorld>>& AvailableTemplates)
{
	Templates = AvailableTemplates;
}

void AMatchSetup::ProcessBattleStart()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMatchSetup::ProcessBattleStart)

	FPlayerData* DefenderData = GetNewDefender();
	if (!DefenderData->ArenaActor.IsValid() || DefenderData == nullptr) return;

	DefenderData->bWasDefender = true;
	
	SetCurrentAttackerStartSpots(DefenderData->ArenaActor->GetAttackerPlayerStarts());

	AssignTeamsByDefender(DefenderData->PlayerController);
}

void AMatchSetup::ProcessBattleEnd()
{
	RemoveAllTeams();
}

void AMatchSetup::CacheArenaActors()
{
	for (TWeakObjectPtr<AArenaActor> ArenaActor : TActorRange<AArenaActor>(GetWorld()))
	{
		ArenaArray.Add(ArenaActor);
	}
}

TWeakObjectPtr<AArenaActor> AMatchSetup::GetAssignedArenaActor(const TWeakObjectPtr<APlayerController> Controller)
{
	for (const FPlayerData& Data : MatchData)
	{
		if (Data.PlayerController == Controller)
		{
			return Data.ArenaActor;
		}
	}
	return nullptr;
}

TWeakObjectPtr<ULevelStreamingDynamic> AMatchSetup::StreamDynamicLevel(const FPlayerData& Data)
{
	if (Data.LoadedTemplate != nullptr)
	{
		Data.LoadedTemplate->SetIsRequestingUnloadAndRemoval(true);
	}
	
	bool bSuccess = false;
	const TObjectPtr<ULevelStreamingDynamic> OutLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		this,
		Data.WorldToStream,
		Data.ArenaActor->GetActorTransform(),
		bSuccess,
		MakeLevelStreamingName(Data.GetUniqueId()),
		nullptr,
		true);

	if (Data.PlayerController.IsValid() && Data.PlayerController->IsLocalController())
	{
		Data.PlayerController->OnNewTemplateLoadAction.Broadcast(OutLevel);
	}
	return OutLevel;
}

void AMatchSetup::LoadNextTemplate(const TWeakObjectPtr<APlayerController> PlayerController)
{
	FPlayerData* Data = GetMatchDataForPlayer(PlayerController);
	if (Data == nullptr)
	{
		return;
	}
	int32 CurrentIndex = GetCurrentTemplateIndex(Data->WorldToStream);
	CurrentIndex++;
	
	if (CurrentIndex >= Templates.Num())
	{
		CurrentIndex = 0;
	}
	UpdateWorldToStream(*Data, CurrentIndex);
}

void AMatchSetup::LoadPreviousTemplate(const TWeakObjectPtr<APlayerController> PlayerController)
{
	FPlayerData* Data = GetMatchDataForPlayer(PlayerController);
	if (Data == nullptr)
	{
		return;
	}

	int32 CurrentIndex = GetCurrentTemplateIndex(Data->WorldToStream);
	CurrentIndex--;
	
	if (CurrentIndex < 0)
	{
		CurrentIndex = Templates.Num() - 1;
	}
	UpdateWorldToStream(*Data, CurrentIndex);
}

TArray<TObjectPtr<APlayerStart>> AMatchSetup::GetCurrentAttackerStartSpots() const
{
	return CurrentAttackerStartSpots;
}

void AMatchSetup::OnRep_MatchData()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMatchSetup::OnRep_MatchData)
	
	for (FPlayerData& Data : MatchData)
	{
		if (Data.ArenaActor.IsValid())
		{
			if (Data.HasUniqueIdChanged() && !Data.WorldToStream.IsNull())
			{
				Data.UpdateLastId();
				Data.LoadedTemplate = StreamDynamicLevel(Data);
			}
		}
	}
}
