// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ToyboxGameMode.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "MatchGameMode.generated.h"

class APlayerStart;
class UToyboxAbilitySet;
class AMatchGameState;
class AMatchSetup;


namespace BattleState
{
	extern TOYBOX_API const FName PreGame;
	extern TOYBOX_API const FName Preparation;
	extern TOYBOX_API const FName Battle;
	extern TOYBOX_API const FName PostBattle;
	extern TOYBOX_API const FName PostGame;
}

/**
 * 
 */
UCLASS()
class TOYBOX_API AMatchGameMode : public AToyboxGameMode
{
	GENERATED_BODY()

public:
	AMatchGameMode();
	
	//Match State
	virtual void HandleMatchIsWaitingToStart() override;
	void StartPreparation();

	virtual void HandleMatchHasStarted() override;

	virtual void HandleMatchHasEnded() override;

	virtual void HandleLeavingMap() override;
	//Match State End

	void GetAllPossibleStartSpots(AController* Player, TArray<TWeakObjectPtr<APlayerStart>>& UnOccupiedStartPoints, TArray<TWeakObjectPtr<APlayerStart>>& OccupiedStartPoints);

	
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
	//Battle State
public:
	UFUNCTION(BlueprintCallable, Category="Game")
	FName GetBattleState() const { return BattleState; }

protected:
	/** What match state we are currently in */
	UPROPERTY(Transient)
	FName BattleState;
	
	virtual void SetBattleState(FName NewState);
	
	/** Implementable event to respond to match state changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Game", meta=(DisplayName="OnSetBattleState", ScriptName="OnSetBattleState"))
	void K2_OnBattleStateSet(FName NewState);
	
	virtual bool HandleEndPreGame();
	
	virtual bool HandleStartPreparation();

	virtual bool HandleEndPreparation();

	virtual bool HandleStartBattle();

	virtual bool HandleEndBattle();

	virtual bool HandleStartPostBattle();

	virtual bool HandleEndPostBattle();

	virtual bool HandleStartPostGame();

	virtual bool HandleEndPostGame();
	
	
	//Battle State End
public:
	//Players Starting/Init of Game
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	virtual void PreInitializeComponents() override;

	virtual void InitGameState() override;

	virtual void PostSeamlessTravel() override;

	virtual void SetSeamlessTravelViewTarget(APlayerController* PC) override;

	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

	virtual void InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer) override;

	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;
	//Players Starting/Init of Game
protected:
	virtual bool ReadyToStartMatch_Implementation() override;

	virtual void InitSeamlessTravelPlayer(AController* NewController) override;

	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;

	void RestartAllPlayers();

	void DestroyAllPlayerPawns();

	static void DestroyPlayerPawn(TWeakObjectPtr<AController> PlayerController);

protected:	
	UPROPERTY(EditDefaultsOnly, Category = "Match|TemplateSelection")
	TArray<TSoftObjectPtr<UWorld>> Templates;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0, UIMin = 0), Category = "Match|Timer")
	float TemplateChoiceTime = 35.0f;
	
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0, UIMin = 0), Category = "Match|Timer")
	float PreparationTime = 120.0f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0, UIMin = 0), Category = "Match|Timer")
	float BattleTime = 300.0f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0, UIMin = 0), Category = "Match|Timer")
	float TimeBetweenBattles = 20.0f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0, UIMin = 0), Category = "Match|Timer")
	float TimeBeforeReturningToMainMenu = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Match|Timer")
	TObjectPtr<UCurveFloat> AttackerRespawnDelay = {nullptr};

private:
	TObjectPtr<AMatchSetup> MatchSetup = {nullptr};

	TWeakObjectPtr<AMatchGameState> MatchGameState = {nullptr};

	bool bBattleStateChangeIsPending = false;
public:
	float GetMaxTimeTemplateChoice() const;

	float GetPreparationTime() const;

	float GetBattleTime() const;

	float GetTimeBetweenBattles() const;

	float GetTimeBeforeReturningToMainMenu() const;

	void StartBattle();

	float GetRespawnDelay(const int32 DeathCount) const;

	void NotifyBattleIsOver();
	void StartPostGame();

	void NotifyPostBattleIsOver();

	void NotifyPostGameIsOver();
};