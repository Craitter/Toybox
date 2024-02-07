// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HelperObjects/ToyboxDataTypes.h"
#include "LobbyPlayerInfoWidget.generated.h"

enum class EChampion;
enum class EToyline;
class UTextBlock;
/**
 * 
 */
UCLASS()
class TOYBOX_API ULobbyPlayerInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameInfo = {nullptr};
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ToylineInfo = {nullptr};
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ChampionInfo = {nullptr};
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerReadyInfo = {nullptr};

public:
	void SetPlayerNameInfo(const FText& Name) const;
	void SetToylineInfo(const EToyline Toyline) const;
	void SetChampionInfo(const EChampion Champion) const;
	void SetPlayerReadyInfo(const bool bIsReady) const;
	void ResetPlayerInfo() const;
};
