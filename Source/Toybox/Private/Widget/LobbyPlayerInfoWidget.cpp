// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/LobbyPlayerInfoWidget.h"

#include "Components/TextBlock.h"

void ULobbyPlayerInfoWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!ensure(IsValid(PlayerNameInfo))) return;
	if (!ensure(IsValid(ToylineInfo))) return;
	if (!ensure(IsValid(ChampionInfo))) return;
	if (!ensure(IsValid(PlayerReadyInfo))) return;
}

void ULobbyPlayerInfoWidget::SetPlayerNameInfo(const FText& Name) const
{
	PlayerNameInfo->SetText(Name);
}

void ULobbyPlayerInfoWidget::SetToylineInfo(const EToyline Toyline) const
{	
	ToylineInfo->SetText(UEnum::GetDisplayValueAsText(Toyline));
}

void ULobbyPlayerInfoWidget::SetChampionInfo(const EChampion Champion) const
{
	ChampionInfo->SetText(UEnum::GetDisplayValueAsText(Champion));
}

void ULobbyPlayerInfoWidget::SetPlayerReadyInfo(const bool bIsReady) const
{
	const FString Ready = bIsReady ? FString(TEXT("Ready")) : FString(TEXT("Not Ready"));
	PlayerReadyInfo->SetText(FText::FromString(Ready));
}

void ULobbyPlayerInfoWidget::ResetPlayerInfo() const
{
	PlayerNameInfo->SetText(FText::GetEmpty());
	ToylineInfo->SetText(FText::GetEmpty());
	ChampionInfo->SetText(FText::GetEmpty());
	PlayerReadyInfo->SetText(FText::GetEmpty());
}
