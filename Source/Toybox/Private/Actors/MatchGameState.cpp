// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MatchGameState.h"

#include "Actors/MatchGameMode.h"
#include "Actors/MatchPlayerState.h"
#include "Actors/MatchSetup.h"
#include "Actors/ToyboxPlayerState.h"
#include "Framework/ToyboxGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Toybox/Toybox.h"


AMatchGameState::AMatchGameState()
{
	bReplicates = true;
}

void AMatchGameState::ReceivedGameModeClass()
{
	Super::ReceivedGameModeClass();

	ToyboxAuthorityGameMode = Cast<AMatchGameMode>(AuthorityGameMode);
}

void AMatchGameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	//Server handles this in GameMode
	if (IsValid(ToyboxAuthorityGameMode))
	{
		SetCountdown(ToyboxAuthorityGameMode->GetMaxTimeTemplateChoice());
	}
}

void AMatchGameState::HandleLeavingMap()
{
	Super::HandleLeavingMap();
	
	const TWeakObjectPtr<UToyboxGameInstance> GameInstance = GetGameInstance<UToyboxGameInstance>();
	if (!ensure(GameInstance.IsValid())) return;

	GameInstance->BackToMainMenu();
}

FName AMatchGameState::GetBattleState() const
{
	return OldBattleState;
}

bool AMatchGameState::IsGameModeInBattle() const
{
	return ToyboxAuthorityGameMode->GetBattleState() == BattleState::Battle;
}

bool AMatchGameState::IsGameModeInPreparation() const
{
	return ToyboxAuthorityGameMode->GetBattleState() == BattleState::Preparation;
}

void AMatchGameState::SetBattleState(FName NewState)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		BattleState = NewState;

		// Call the onrep to make sure the callbacks happen
		OnRep_BattleState();
	}
}

void AMatchGameState::AttackerDied(const TWeakObjectPtr<APlayerController> PlayerController, const int32 CurrentDeathCount)
{
	FTimerHandle& Handle = RespawnTimers.Add(PlayerController);
	const float RespawnDelay = ToyboxAuthorityGameMode->GetRespawnDelay(CurrentDeathCount);
	GetWorldTimerManager().SetTimer(Handle, this, &ThisClass::OnRespawnTimerExpired, RespawnDelay, false);
}

void AMatchGameState::DefenderDied(const TWeakObjectPtr<APlayerController> PlayerController) const
{
	ToyboxAuthorityGameMode->NotifyBattleIsOver();
}

void AMatchGameState::GetAllAttackers(TArray<TWeakObjectPtr<AMatchPlayerState>>& Attackers)
{
	for (TWeakObjectPtr<APlayerState> PlayerState : PlayerArray)
	{
		TWeakObjectPtr<AMatchPlayerState> ToyboxPlayerState = Cast<AMatchPlayerState>(PlayerState);
		if (ToyboxPlayerState.IsValid() && ToyboxPlayerState->IsAttackerTeam())
		{
			Attackers.Add(ToyboxPlayerState);
		}
	}
}

bool AMatchGameState::IsLocalPlayerDefender() const
{
	const TWeakObjectPtr<AMatchPlayerState> Defender = GetDefender();

	if (!Defender.IsValid())
	{
		return false;
	}
	
	const TWeakObjectPtr<APlayerController> PlayerController = Defender->GetPlayerController();
	if (PlayerController.IsValid())
	{
		return PlayerController->IsLocalController();
	}
	
	return false;
}

TWeakObjectPtr<AMatchPlayerState> AMatchGameState::GetDefender() const
{
	for (TWeakObjectPtr<APlayerState> PlayerState : PlayerArray)
	{
		TWeakObjectPtr<AMatchPlayerState> ToyboxPlayerState = Cast<AMatchPlayerState>(PlayerState);
		if (ToyboxPlayerState.IsValid() && ToyboxPlayerState->IsDefenderTeam())
		{
			return ToyboxPlayerState.Get();
		}
	}
	return nullptr;
}

UAbilitySystemComponent* AMatchGameState::GetDefenderAbilitySystemComponent() const
{
	const TWeakObjectPtr<AMatchPlayerState> Defender = GetDefender();
	if (Defender.IsValid())
	{
		return Defender->GetAbilitySystemComponent();
	}
	
	return nullptr;
}

void AMatchGameState::HandleEndPreGame()
{
}

void AMatchGameState::HandleStartPreparation()
{
	if (HasAuthority())
	{
		StopCountdown();
		SetCountdown(ToyboxAuthorityGameMode->GetPreparationTime());
	}
}

void AMatchGameState::HandleEndPreparation()
{
	if (HasAuthority())
	{
		StopCountdown();
	}
}

void AMatchGameState::HandleStartBattle()
{
	if (HasAuthority())
	{
		SetCountdown(ToyboxAuthorityGameMode->GetBattleTime());
	}
}

void AMatchGameState::HandleEndBattle()
{
	if (HasAuthority())
	{
		StopCountdown();
		ForceClearRespawnTimers();
	}
}

void AMatchGameState::HandleStartPostBattle()
{
	if (HasAuthority())
	{
		SetCountdown(ToyboxAuthorityGameMode->GetTimeBetweenBattles());
	}
}

void AMatchGameState::HandleEndPostBattle()
{
	if (HasAuthority())
	{
		StopCountdown();
	}
}

void AMatchGameState::HandleStartPostGame()
{
	if (HasAuthority())
	{
		SetCountdown(ToyboxAuthorityGameMode->GetTimeBeforeReturningToMainMenu());
	}
}

void AMatchGameState::HandleEndPostGame()
{
	if (HasAuthority())
	{
		StopCountdown();
	}
}

void AMatchGameState::OnRep_BattleState()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMatchGameState::OnRep_BattleState)

	if (BattleState == OldBattleState)
	{
		return;
	}
	// Call change callbacks
	if (OldBattleState == BattleState::PreGame)
	{
		HandleEndPreGame();
	}
	else if (OldBattleState == BattleState::Preparation)
	{
		HandleEndPreparation();
	}
	else if (OldBattleState == BattleState::Battle)
	{
		HandleEndBattle();
	}
	else if (OldBattleState == BattleState::PostBattle)
	{
		HandleEndPostBattle();
	}
	else if (OldBattleState == BattleState::PostGame)
	{
		HandleEndPostBattle();
	}

	UE_LOG(LogGameState, Display, TEXT("Battle State Changed from %s to %s"), *OldBattleState.ToString(), *BattleState.ToString());

	OldBattleState = BattleState;
	
	
	if (BattleState == BattleState::Preparation)
	{
		HandleStartPreparation();
	}
	else if (BattleState == BattleState::Battle)
	{
		HandleStartBattle();
	}
	else if (BattleState == BattleState::PostBattle)
	{
		HandleStartPostBattle();
	}
	else if (BattleState == BattleState::PostGame)
	{
		HandleStartPostGame();
	}

	OnBattleStateChanged.Broadcast(BattleState);
}

void AMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, MatchSetup);

	DOREPLIFETIME(ThisClass, BattleState);
}

void AMatchGameState::OnRespawnTimerExpired()
{
	TArray<TWeakObjectPtr<APlayerController>> ControllersToRestart;
	for (const TTuple<TWeakObjectPtr<APlayerController>, FTimerHandle>& KeyValuePair : RespawnTimers)
	{
		if (GetWorldTimerManager().GetTimerRemaining(KeyValuePair.Value) < UE_FLOAT_NORMAL_THRESH)
		{
			ControllersToRestart.Add(KeyValuePair.Key);
		}
	}
	for (TWeakObjectPtr<APlayerController> PlayerController : ControllersToRestart)
	{
		ToyboxAuthorityGameMode->RestartPlayer(PlayerController.Get());
		RespawnTimers.Remove(PlayerController);
	}
}

void AMatchGameState::ForceClearRespawnTimers()
{
	for (TTuple<TWeakObjectPtr<APlayerController>, FTimerHandle>& KeyValuePair : RespawnTimers)
	{
		GetWorldTimerManager().ClearTimer(KeyValuePair.Value);
	}
	RespawnTimers.Reset();
}

void AMatchGameState::TogglePlayerIsReady(APlayerState* OwningPlayerState)
{
	Super::TogglePlayerIsReady(OwningPlayerState);

	//Once a Player Confirms to be ready we will hide the confirm button so you cant unready
	PlayerReadyCount++;
	//this calls Can StartMatch, so we can just check there if conditions are met
	ToyboxAuthorityGameMode->StartPlay();
}

void AMatchGameState::OnCountdownExpired()
{
	Super::OnCountdownExpired();
	
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		ToyboxAuthorityGameMode->StartMatch();
	}
	else if (GetBattleState() == BattleState::Preparation)
	{
		ToyboxAuthorityGameMode->StartBattle();
	}
	else if (GetBattleState() == BattleState::Battle)
	{
		ToyboxAuthorityGameMode->NotifyBattleIsOver();
	}
	else if (GetBattleState() == BattleState::PostBattle)
	{
		ToyboxAuthorityGameMode->NotifyPostBattleIsOver();
	}
	else if (GetBattleState() == BattleState::PostGame)
	{
		ToyboxAuthorityGameMode->NotifyPostGameIsOver();
	}
}

