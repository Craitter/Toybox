// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Actors/MatchPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "OverlayWidget.generated.h"

class UAbilityWidget;
class UAmmoWidget;
class AToyboxPlayerState;
class UTextBlock;
class UCrosshairWidget;
class UDynamicEntryBox;
class UHealthWidget;
/**
 * 
 */
UCLASS()
class TOYBOX_API UOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHealthWidget> OwningPawnHealthWidget = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHealthWidget> DefenderHealthWidget = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAmmoWidget> AmmoWidget = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> TeamsHealth = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCrosshairWidget> CrosshairWidget = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAbilityWidget> ThrowableWidget = {nullptr};
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAbilityWidget> SpecialWidget = {nullptr};

	TWeakObjectPtr<AMatchGameState> MatchGameState = {nullptr};

	void OnTeamTagChanged(FGameplayTag ChangedTeamTag, const int32 NewCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Widget|TeamTag", DisplayName = "TeamTagChanged", meta=(ScriptName = "TeamTagChanged"))
	void K2_OnTeamTagChanged(FGameplayTag NewTeamTag);
	
private:
	void UpdateTeamHealthWidget() const;
	void UpdateDefenderHealthWidget() const;

	void CreateWidgetsForAttackers(TArray<TWeakObjectPtr<AMatchPlayerState>>& Attackers) const;

	void OnNewAttacker(TWeakObjectPtr<AMatchPlayerState> PlayerState) const;
	void OnNewDefender(TWeakObjectPtr<AMatchPlayerState> PlayerState) const;

	void InitializeAbilityWidgets() const;
	
	void InitializeHealthWidget() const;
	
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = {nullptr};
};
