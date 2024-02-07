// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ToyboxGameState.h"

#include "Net/UnrealNetwork.h"
#include "Toybox/Toybox.h"


void AToyboxGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CountdownEndServerTimeSeconds);
}

float AToyboxGameState::GetServerWorldTimeSecondsUpdateFrequency() const
{
	return ServerWorldTimeSecondsUpdateFrequency;
}

void AToyboxGameState::OnRep_CountdownEndServerTimeSeconds() const
{	
	if (CountdownEndServerTimeSeconds > GetServerWorldTimeSeconds())
	{
		if (!OnNewCountdownTimerSetDelegate.IsBound())
		{
			UE_LOG(LogTemp, Log, TEXT("CountdownTimerDelegate is not bound on %s"), *UEnum::GetValueAsString(GetLocalRole()));
			return;
		}
		OnNewCountdownTimerSetDelegate.Execute(CountdownEndServerTimeSeconds);
	}
}

void AToyboxGameState::StopCountdown()
{
	if (CountdownTimerHandle.IsValid())
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(AToyboxGameState::StopCountdown)
		
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		OnCountdownExpired();
	}
}

bool AToyboxGameState::IsCountdownActive() const
{
	return CountdownEndServerTimeSeconds > GetServerWorldTimeSeconds();
}

double AToyboxGameState::GetCountdownEndServerTimeSeconds() const
{
	return CountdownEndServerTimeSeconds;
}

void AToyboxGameState::SetCountdown(const double Seconds)
{
	check(GetWorld());
	SetCountdownEndServerTimeSeconds(GetWorld()->GetTimeSeconds() + Seconds);
	
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ThisClass::OnCountdownExpired, Seconds, false);
}

void AToyboxGameState::OnCountdownExpired()
{
	CountdownTimerHandle.Invalidate();
	if (!OnCountdownExpiredDelegate.IsBound())
	{
		UE_LOG(LogTemp, Log, TEXT("OnCountdownExpiredDelegate is not bound on %s"), *UEnum::GetValueAsString(GetLocalRole()));
		return;
	}
	OnCountdownExpiredDelegate.Execute();
}

void AToyboxGameState::SetCountdownEndServerTimeSeconds(const double NewCountdownServerTime)
{
	CountdownEndServerTimeSeconds = NewCountdownServerTime;
	OnRep_CountdownEndServerTimeSeconds();
}
