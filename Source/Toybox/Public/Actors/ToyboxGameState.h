// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "HelperObjects/ToyboxDataTypes.h"

#include "ToyboxGameState.generated.h"

/**
 * 
 */
DECLARE_DELEGATE_OneParam(FNewCountdownTimerSet, double);
DECLARE_DELEGATE(FCountdownExpired);

UCLASS()
class TOYBOX_API AToyboxGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	virtual void TogglePlayerIsReady(APlayerState* OwningPlayerState) {}

	//The variable is protected, but we need it in the widget, so we create getter,
	//returns the replicated ServerWorldTimeSeconds Update frequency (its calculated)
	float GetServerWorldTimeSecondsUpdateFrequency() const;
	
	//Timer
	FNewCountdownTimerSet OnNewCountdownTimerSetDelegate;
	FCountdownExpired OnCountdownExpiredDelegate;
	FTimerHandle CountdownTimerHandle;
	//Sets and start replication of the Countdown
	void SetCountdown(double Seconds);
	//Can be overriden to make custom logic happen when the Countdown expires
	virtual void OnCountdownExpired();
	
	double GetCountdownEndServerTimeSeconds() const;
	bool IsCountdownActive() const;

protected:
	//Setter to trigger OnRep
	void SetCountdownEndServerTimeSeconds(double NewCountdownServerTime);

	UFUNCTION()
	void OnRep_CountdownEndServerTimeSeconds() const;

	void StopCountdown();
	
private:
	//Replicated property to let clients know when a countdown ends (same time as server)
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_CountdownEndServerTimeSeconds)
	double CountdownEndServerTimeSeconds;

	//Timer End
};
