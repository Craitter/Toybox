// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

struct FGameplayTag;
struct FToyboxGameplayTags;
class UImage;
/**
 * 
 */
UCLASS()
class TOYBOX_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	
protected:
	void DisableCrosshair() const;

	FOnGameplayEffectTagCountChanged::FDelegate OnRangedTagChangedDelegate;
	FOnGameplayEffectTagCountChanged::FDelegate OnMeleeTagChangedDelegate;
	FOnGameplayEffectTagCountChanged::FDelegate OnUnarmedTagChangedDelegate;
	
	void OnWeaponTagChanged(FGameplayTag Weapon, int32 TagCount) const;

	void EnableCrosshair(FGameplayTag CrosshairToEnable) const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Crosshair = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> RangedImage = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> MeleeImage = {nullptr};
};
