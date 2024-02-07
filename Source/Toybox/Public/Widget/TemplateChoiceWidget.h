// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/GameState.h"
#include "TemplateChoiceWidget.generated.h"

class AMatchSetup;
class UCircularThrobber;
class AToyboxPlayerController;
class AMatchGameState;
class UButton;
/**
 * 
 */
UCLASS()
class TOYBOX_API UTemplateChoiceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;

	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextTemplateButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PreviousTemplateButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmTemplateButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCircularThrobber> LoadingThrobber = {nullptr};

	UFUNCTION()
	void OnNextTemplateButtonPressed();

	UFUNCTION()
	void OnPreviousTemplateButtonPressed();

	UFUNCTION()
	void OnConfirmTemplateButtonPressed();

	void OnNewTemplateCreation(TWeakObjectPtr<ULevelStreamingDynamic> NewTemplate);

	UFUNCTION()
	void OnTemplateShown();
	UFUNCTION()
	void OnTemplateHidden();
	

	TWeakObjectPtr<AToyboxPlayerController> PlayerController = {nullptr};

	TWeakObjectPtr<AMatchSetup> MatchSetup = {nullptr};
};
