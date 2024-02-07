// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/CrosshairWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/Image.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"

void UCrosshairWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	DisableCrosshair();

	const TWeakObjectPtr<APawn> OwningPawn = GetOwningPlayerPawn();

	if (!OwningPawn.IsValid())
	{
		return;
	}

	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwningPawn);
	if (AbilitySystemInterface == nullptr)
	{
		return;
	}
	
	const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = Cast<UAbilitySystemComponent>(
		AbilitySystemInterface->GetAbilitySystemComponent());
	
	if (!AbilitySystemComponent.IsValid())
	{
		return;
	}

	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();

	OnMeleeTagChangedDelegate.BindUObject(this, &ThisClass::OnWeaponTagChanged);
	OnRangedTagChangedDelegate.BindUObject(this, &ThisClass::OnWeaponTagChanged);
	OnUnarmedTagChangedDelegate.BindUObject(this, &ThisClass::OnWeaponTagChanged);
	
	AbilitySystemComponent->RegisterAndCallGameplayTagEvent(NativeTags.Weapon_Ranged, OnRangedTagChangedDelegate, EGameplayTagEventType::NewOrRemoved);
	AbilitySystemComponent->RegisterAndCallGameplayTagEvent(NativeTags.Weapon_Melee, OnMeleeTagChangedDelegate, EGameplayTagEventType::NewOrRemoved);
	AbilitySystemComponent->RegisterAndCallGameplayTagEvent(NativeTags.Weapon_Unarmed, OnUnarmedTagChangedDelegate, EGameplayTagEventType::NewOrRemoved);
}

void UCrosshairWidget::DisableCrosshair() const
{
	if (IsValid(Crosshair))
	{
		Crosshair->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UCrosshairWidget::OnWeaponTagChanged(FGameplayTag Weapon, int32 TagCount) const
{
	if (TagCount > 0)
	{
		DisableCrosshair();
		EnableCrosshair(Weapon);
	}
}

void UCrosshairWidget::EnableCrosshair(const FGameplayTag CrosshairToEnable) const
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	if (!IsValid(Crosshair))
	{
		return;
	}
	
	if (CrosshairToEnable == NativeTags.Weapon_Melee && IsValid(MeleeImage)) 
	{
		Crosshair->SetBrushFromTexture(MeleeImage, false);
		Crosshair->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else if (CrosshairToEnable == NativeTags.Weapon_Ranged && IsValid(RangedImage))
	{
		Crosshair->SetBrushFromTexture(RangedImage, false);
		Crosshair->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else if (CrosshairToEnable == NativeTags.Weapon_Unarmed)
	{
		DisableCrosshair();
	}
}
