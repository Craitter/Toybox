// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MatchPlayerState.h"
#include "Actors/ToyboxGameState.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "MatchGameState.generated.h"

class AToyboxPlayerState;
class UTimelineComponent;
class AMatchSetup;
class AMatchGameMode;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBattleStateChanged, FName);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNewTeam, TWeakObjectPtr<AMatchPlayerState>);

/**
 * 
 */
UCLASS()
class TOYBOX_API AMatchGameState : public AToyboxGameState
{
	GENERATED_BODY()
		
public:
	AMatchGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void ReceivedGameModeClass() override;

	//Match State
	virtual void HandleMatchIsWaitingToStart() override;

	virtual void HandleLeavingMap() override;
	//Match State End

	//Battle State
	FName GetBattleState() const;

	bool IsGameModeInBattle() const;

	bool IsGameModeInPreparation() const;

	/** Updates the match state and calls the appropriate transition functions, only valid on server */
	void SetBattleState(FName NewState);

	void AttackerDied(const TWeakObjectPtr<APlayerController> PlayerController, int32 CurrentDeathCount);

	void DefenderDied(const TWeakObjectPtr<APlayerController> PlayerController) const;

	void GetAllAttackers(TArray<TWeakObjectPtr<AMatchPlayerState>>& Attackers);

	bool IsLocalPlayerDefender() const;
	
	TWeakObjectPtr<AMatchPlayerState> GetDefender() const;

	UFUNCTION(BlueprintCallable)
	UAbilitySystemComponent* GetDefenderAbilitySystemComponent() const;

	FOnNewTeam OnNewAttacker;

	FOnNewTeam OnNewDefender;

protected:
	/** What match state we are currently in */
	UPROPERTY(ReplicatedUsing=OnRep_BattleState, BlueprintReadOnly, VisibleInstanceOnly, Category = GameState)
	FName BattleState;

	FName OldBattleState;

	virtual void HandleEndPreGame();
	
	virtual void HandleStartPreparation();

	virtual void HandleEndPreparation();

	virtual void HandleStartBattle();

	virtual void HandleEndBattle();

	virtual void HandleStartPostBattle();

	virtual void HandleEndPostBattle();

	virtual void HandleStartPostGame(); 
	
	virtual void HandleEndPostGame();

public:
	
	/** Match state has changed */
	UFUNCTION()
	virtual void OnRep_BattleState();


	FOnBattleStateChanged OnBattleStateChanged;
	//Battle State End
	
	UPROPERTY(Replicated)
	TWeakObjectPtr<AMatchSetup> MatchSetup = {nullptr};

	virtual void TogglePlayerIsReady(APlayerState* OwningPlayerState) override;

	int32 PlayerReadyCount = 0;
	
private:
	TObjectPtr<AMatchGameMode> ToyboxAuthorityGameMode = {nullptr};

protected:
	
	virtual void OnCountdownExpired() override;

	TMap<TWeakObjectPtr<APlayerController>, FTimerHandle> RespawnTimers;

	void OnRespawnTimerExpired();
	void ForceClearRespawnTimers();
	
public:
	
};
