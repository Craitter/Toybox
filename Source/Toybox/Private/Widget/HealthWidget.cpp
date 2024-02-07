// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HealthWidget.h"

#include "Actors/MatchPlayerState.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Framework/ToyboxAbilitySystemComponent.h"
#include "Framework/AttributeSets/HealthAttributeSet.h"


void UHealthWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	AttributeNumberFormat.AlwaysSign = false;
	AttributeNumberFormat.MaximumFractionalDigits = 0;
	AttributeNumberFormat.MinimumFractionalDigits = 0;
	AttributeNumberFormat.MaximumIntegralDigits = 4;
	AttributeNumberFormat.MinimumIntegralDigits = 1;
}

void UHealthWidget::NativeDestruct()
{	
	Super::NativeDestruct();
	MarkAsGarbage();
}

void UHealthWidget::SetRelatedPlayerState(const TWeakObjectPtr<APlayerState> PlayerState)
{
	RelatedPlayerState = Cast<AMatchPlayerState>(PlayerState);
	
	if (!RelatedPlayerState.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Passed Playerstate to health Widget was not valid"));
		return;
	}

	const FChampionData* ChampionData = RelatedPlayerState->GetChampionData();

	if (IsValid(ChampionData->CharacterIcon))
	{
		PlayerIcon->SetBrushFromTexture(ChampionData->CharacterIcon);
	}
	else
	{
		PlayerIcon->SetVisibility(ESlateVisibility::Hidden);
	}
	
	RelatedPlayerState->OnCharacterDataInitialized.AddUObject(this, &ThisClass::OnCharacterDataChanged);
	OnCharacterDataChanged(RelatedPlayerState->GetCharacterData());
}

void UHealthWidget::InitializeHealthWidget()
{	
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(RelatedPlayerState);
	const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = AbilitySystemInterface->GetAbilitySystemComponent();

	
	const UHealthAttributeSet* HealthAttributeSet = Cast<UHealthAttributeSet>(
		AbilitySystemComponent->GetAttributeSet(UHealthAttributeSet::StaticClass()));
	if (HealthAttributeSet == nullptr)
	{
		return;
	}

	PlayerName->SetText(FText::FromString(FString(RelatedPlayerState->GetPlayerName())));
	
	HealthChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		HealthAttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
	
	ShieldChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		HealthAttributeSet->GetShieldAttribute()).AddUObject(this, &ThisClass::OnShieldChanged);
	
	MaxHealth = HealthAttributeSet->GetMaxHealth();
	MaxShield = HealthAttributeSet->GetMaxShield();

	
	SetCurrentHealth(HealthAttributeSet->GetHealth());
	SetCurrentShield(HealthAttributeSet->GetShield());
}

void UHealthWidget::OnHealthChanged(const FOnAttributeChangeData& Data) const
{
	SetCurrentHealth(Data.NewValue);
}

void UHealthWidget::OnShieldChanged(const FOnAttributeChangeData& Data) const
{
	SetCurrentShield(Data.NewValue);
}

void UHealthWidget::SetCurrentHealth(const float NewHealth) const
{
	if (HealthPoints == nullptr || HealthBar == nullptr)
	{
		return;
	}
	HealthPoints->SetText(FText::AsNumber(NewHealth, &AttributeNumberFormat));

	if (NewHealth <= UE_FLOAT_NORMAL_THRESH)
	{
		HealthBar->SetPercent(0);
		return;
	}
	HealthBar->SetPercent(NewHealth / MaxHealth);
}

void UHealthWidget::SetCurrentShield(const float NewShield) const
{
	if (ShieldPoints == nullptr || ShieldBar == nullptr)
	{
		return;
	}
	ShieldPoints->SetText(FText::AsNumber(NewShield, &AttributeNumberFormat));

	if (NewShield <= UE_FLOAT_NORMAL_THRESH)
	{
		ShieldBar->SetPercent(0);
		return;
	}
	ShieldBar->SetPercent(NewShield / MaxShield);
}

void UHealthWidget::OnCharacterDataChanged(TWeakObjectPtr<UToyboxCharacterData> NewCharacterData)
{
	if (NewCharacterData != nullptr)
	{
		InitializeHealthWidget();
	}
}
