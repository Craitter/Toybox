// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/CountdownWidget.h"

#include "Actors/ToyboxGameState.h"
#include "Components/TextBlock.h"

const FText TimeFormat = FText::FromString(FString("{Min}:{Sec}"));
const FText TimeFormatOnlySec = FText::FromString(FString("{Sec}"));

void UCountdownWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	NumberFormattingOptions.MinimumIntegralDigits = 2;
	NumberFormattingOptions.MaximumIntegralDigits = 2;
	NumberFormattingOptions.MinimumFractionalDigits = 0;
	NumberFormattingOptions.MaximumFractionalDigits = 0;
	NumberFormattingOptions.RoundingMode = HalfToZero;
	NumberFormattingOptions.AlwaysSign = false;	
	
	check(GetWorld())
	if (!IsValid(TimerDisplay)) return;
	TimerDisplay->SetVisibility(ESlateVisibility::Collapsed);
	
	GetWorld()->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateSet);
	
	const TWeakObjectPtr<AGameStateBase> GameStateBase = GetWorld()->GetGameState<AGameStateBase>();
	if (GameStateBase.IsValid())
	{
		OnGameStateSet(GameStateBase.Get());
	}
}

void UCountdownWidget::OnCountdownSet(const double DesiredServerWorldTime)
{
	checkf(GetWorld(), TEXT("World was null UCountdownWidget::OnCountdownSet"))

	if (UpdateCountdownTimerHandle.IsValid())
	{
		RemoveTimerDisplay();
	}
	
	TargetWorldServerTime = DesiredServerWorldTime;
	if (GameState.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(UpdateCountdownTimerHandle, this, &ThisClass::UpdateCountdown, GameState->GetServerWorldTimeSecondsUpdateFrequency(), true);
	}
	UpdateCountdown();
	if (IsValid(TimerDisplay))
	{
		TimerDisplay->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UCountdownWidget::UpdateCountdown()
{
	check(GetWorld());
	
	if (!ensure(GameState.IsValid())) return;
	if (!ensure(IsValid(TimerDisplay))) return;
	
	double RemainingTime = TargetWorldServerTime - GameState->GetServerWorldTimeSeconds();
	if (RemainingTime <= 0)
	{
		//removing the update timer and set a new one, to keep the 0 alive for 1 second then hide the TimerDisplay
		GetWorld()->GetTimerManager().ClearTimer(UpdateCountdownTimerHandle);
		RemainingTime = 0;
	}
	const FTimespan NewTimeSpan = FTimespan::FromSeconds(RemainingTime);

	const int32 Minutes = NewTimeSpan.GetMinutes();
	const int32 Seconds = NewTimeSpan.GetSeconds();
	if (Minutes > 0)
	{
		FFormatNamedArguments Args;
		Args.Add("Min", FText::AsNumber(Minutes, &NumberFormattingOptions));
		Args.Add("Sec", FText::AsNumber(Seconds, &NumberFormattingOptions));
	
		TimerDisplay->SetText(FText::Format(TimeFormat, Args));
	}
	else
	{
		FFormatNamedArguments Args;
		Args.Add("Sec", FText::AsNumber(Seconds, &NumberFormattingOptions));
	
		TimerDisplay->SetText(FText::Format(TimeFormatOnlySec, Args));
	}
	
}

void UCountdownWidget::RemoveTimerDisplay()
{
	check(GetWorld())
	
	if (!IsValid(TimerDisplay)) return;
	TimerDisplay->SetVisibility(ESlateVisibility::Collapsed);
	
	GetWorld()->GetTimerManager().ClearTimer(UpdateCountdownTimerHandle);
}

void UCountdownWidget::OnGameStateSet(AGameStateBase* NewGameState)
{
	GameState = Cast<AToyboxGameState>(NewGameState);
	if (!GameState.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Countdown Widget does not work, since the GameState of this world is not AToyboxGameState"));
		return;
	}
	GameState->OnNewCountdownTimerSetDelegate.BindUObject(this, &ThisClass::OnCountdownSet);
	GameState->OnCountdownExpiredDelegate.BindUObject(this, &ThisClass::RemoveTimerDisplay);
	if (GameState->IsCountdownActive())
	{
		OnCountdownSet(GameState->GetCountdownEndServerTimeSeconds());
	}
}
