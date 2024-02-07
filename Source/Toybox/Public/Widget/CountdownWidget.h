// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CountdownWidget.generated.h"

class UTextBlock;
class AToyboxGameState;
/**
 * 
 */
UCLASS()
class TOYBOX_API UCountdownWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	
	void OnCountdownSet(double DesiredServerWorldTime);

	void UpdateCountdown();

	void RemoveTimerDisplay();

	void OnGameStateSet(AGameStateBase* NewGameState);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimerDisplay = {nullptr};

private:
	FTimerHandle UpdateCountdownTimerHandle;

	TWeakObjectPtr<AToyboxGameState> GameState = {nullptr};

	double TargetWorldServerTime = 0.0f;

	FNumberFormattingOptions NumberFormattingOptions;

	bool bFinished = false;
};
