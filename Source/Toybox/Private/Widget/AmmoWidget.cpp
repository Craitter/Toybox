// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/AmmoWidget.h"

#include "Actors/MatchPlayerState.h"
#include "Components/TextBlock.h"
#include "Framework/AttributeSets/RangedWeaponAttributeSet.h"


void UAmmoWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!IsValid(BulletCount))
	{
		return;
	}
	
	const TWeakObjectPtr<AMatchPlayerState> RelatedPlayerState = Cast<AMatchPlayerState>(GetOwningPlayerState());
	
	if (!RelatedPlayerState.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Playerstate of Ammo widget was not a MatchPlayerState cant Init"));
		return;
	}
	RelatedPlayerState->OnCharacterDataInitialized.AddUObject(this, &ThisClass::OnCharacterDataChanged);
	OnCharacterDataChanged(RelatedPlayerState->GetCharacterData());
}

void UAmmoWidget::NativeDestruct()
{
	Super::NativeDestruct();
	MarkAsGarbage();
}

void UAmmoWidget::OnCharacterDataChanged(TWeakObjectPtr<UToyboxCharacterData> NewCharacterData)
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwningPlayerState());
	const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = AbilitySystemInterface->GetAbilitySystemComponent();


	const TWeakObjectPtr<const UWeaponAttributeSet> WeaponAttributeSet = Cast<UWeaponAttributeSet>(
		AbilitySystemComponent->GetAttributeSet(UWeaponAttributeSet::StaticClass()));
	if (WeaponAttributeSet == nullptr)
	{
		return;
	}
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		WeaponAttributeSet->GetCurrentAmmoAttribute()).AddUObject(this, &ThisClass::OnAmmoChanged);

	SetCurrentAmmo(WeaponAttributeSet->GetCurrentAmmo());
}

void UAmmoWidget::OnAmmoChanged(const FOnAttributeChangeData& Data) const
{
	SetCurrentAmmo(Data.NewValue);
}

void UAmmoWidget::SetCurrentAmmo(const float NewAmmo) const
{
	BulletCount->SetText(FText::AsNumber(NewAmmo));
}
