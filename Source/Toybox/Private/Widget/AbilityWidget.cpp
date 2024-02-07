// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/AbilityWidget.h"

#include "AbilitySystemComponent.h"
#include "Actors/MatchPlayerState.h"
#include "Components/Image.h"

void UAbilityWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UAbilityWidget::TryRemoveBindings() const
{
}

void UAbilityWidget::NativeDestruct()
{
	Super::NativeDestruct();
	MarkAsGarbage();
}

void UAbilityWidget::SetRelatedPlayerState(TWeakObjectPtr<APlayerState> PlayerState)
{
	RelatedPlayerState = Cast<AMatchPlayerState>(PlayerState);
	
	if (!RelatedPlayerState.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Passed Playerstate to Ability Widget was not valid"));
		return;
	}
	
	RelatedPlayerState->OnCharacterDataInitialized.AddUObject(this, &ThisClass::OnCharacterDataChanged);
	OnCharacterDataChanged(RelatedPlayerState->GetCharacterData());
}

void UAbilityWidget::InitializeWidget()
{
	AbilityIcon->SetBrushFromTexture(ActiveIcon);
	
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(RelatedPlayerState);
	const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = AbilitySystemInterface->GetAbilitySystemComponent();

	AbilitySystemComponent->RegisterGameplayTagEvent(CooldownTag).AddUObject(this, &ThisClass::OnCooldownTagChanged);
}

void UAbilityWidget::OnCharacterDataChanged(TWeakObjectPtr<UToyboxCharacterData> NewCharacterData)
{
	if (NewCharacterData != nullptr)
	{
		InitializeWidget();
	}
}

void UAbilityWidget::OnCooldownTagChanged(FGameplayTag ChangedTeamTag, const int32 NewCount) const
{
	if (NewCount > 0)
	{
		AbilityIcon->SetBrushFromTexture(InactiveIcon);
	}
	else
	{
		AbilityIcon->SetBrushFromTexture(ActiveIcon);
	}
}

void UAbilityWidget::SetAbilityIcons(TObjectPtr<UTexture2D> Active, TObjectPtr<UTexture2D> Inactive)
{
	ActiveIcon = Active;
	InactiveIcon = Inactive;
}

void UAbilityWidget::SetCooldownTag(FGameplayTag Tag)
{
	CooldownTag = Tag;
}
