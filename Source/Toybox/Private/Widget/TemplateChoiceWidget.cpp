// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/TemplateChoiceWidget.h"

#include "Actors/MatchGameState.h"
#include "Actors/MatchSetup.h"
#include "Actors/ToyboxPlayerController.h"
#include "Actors/ToyboxPlayerState.h"
#include "Components/Button.h"
#include "Components/CircularThrobber.h"

void UTemplateChoiceWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	PlayerController = Cast<AToyboxPlayerController>(GetOwningPlayer());

	
	if (!IsValid(NextTemplateButton)) return;
	if (!IsValid(PreviousTemplateButton)) return;
	if (!IsValid(ConfirmTemplateButton)) return;
	if (!IsValid(LoadingThrobber)) return;
	if (!PlayerController.IsValid()) return;
	
	LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);
	NextTemplateButton->OnClicked.AddDynamic(this, &ThisClass::OnNextTemplateButtonPressed);
	PreviousTemplateButton->OnClicked.AddDynamic(this, &ThisClass::OnPreviousTemplateButtonPressed);
	ConfirmTemplateButton->OnClicked.AddDynamic(this, &ThisClass::OnConfirmTemplateButtonPressed);

	PlayerController->OnNewTemplateLoadAction.AddUObject(this, &ThisClass::OnNewTemplateCreation);
}

void UTemplateChoiceWidget::OnNextTemplateButtonPressed()
{
	if (!PlayerController.IsValid())
	{
		return;
	}
	PlayerController->Server_LoadNextTemplate();
}

void UTemplateChoiceWidget::OnPreviousTemplateButtonPressed()
{
	if (!PlayerController.IsValid())
	{
		return;
	}
	PlayerController->Server_LoadPreviousTemplate();
}

void UTemplateChoiceWidget::OnConfirmTemplateButtonPressed()
{
	const TWeakObjectPtr<AToyboxPlayerState> PlayerState = GetOwningPlayerState<AToyboxPlayerState>();
	if (!PlayerState.IsValid())
	{
		return;
	}
	PlayerState->Server_TogglePlayerIsReady();
	
	NextTemplateButton->SetVisibility(ESlateVisibility::Collapsed);
	PreviousTemplateButton->SetVisibility(ESlateVisibility::Collapsed);
	ConfirmTemplateButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UTemplateChoiceWidget::OnNewTemplateCreation(TWeakObjectPtr<ULevelStreamingDynamic> NewTemplate)
{
	if (NewTemplate.IsValid())
	{
		NewTemplate->OnLevelShown.AddDynamic(this, &ThisClass::OnTemplateShown);
		NewTemplate->OnLevelHidden.AddDynamic(this, &ThisClass::OnTemplateHidden);
	}
}

void UTemplateChoiceWidget::OnTemplateShown()
{
	if (!IsValid(LoadingThrobber)) return;
	LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);
}

void UTemplateChoiceWidget::OnTemplateHidden()
{
	if (!IsValid(LoadingThrobber)) return;
	LoadingThrobber->SetVisibility(ESlateVisibility::HitTestInvisible);
}

