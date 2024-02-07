// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "AbilityWidget.generated.h"

class UImage;
class UToyboxCharacterData;
class AMatchPlayerState;
/**
 * 
 */
UCLASS()
class TOYBOX_API UAbilityWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	void TryRemoveBindings() const;

	virtual void NativeDestruct() override;

	void SetRelatedPlayerState(TWeakObjectPtr<APlayerState> PlayerState);
	
	void InitializeWidget();

	UFUNCTION()
	void OnCharacterDataChanged(TWeakObjectPtr<UToyboxCharacterData> NewCharacterData);

	UFUNCTION()
	void OnCooldownTagChanged(FGameplayTag ChangedTeamTag, const int32 NewCount) const;

	void SetAbilityIcons(TObjectPtr<UTexture2D> ActiveIcon, TObjectPtr<UTexture2D> InactiveIcon);

	void SetCooldownTag(FGameplayTag Tag);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> AbilityIcon = {nullptr};

	UPROPERTY()
	FGameplayTag CooldownTag = FGameplayTag::EmptyTag;
	
	UPROPERTY()
	TObjectPtr<UTexture2D> ActiveIcon = {nullptr};

	UPROPERTY()
	TObjectPtr<UTexture2D> InactiveIcon = {nullptr};
	
private:
	TWeakObjectPtr<AMatchPlayerState> RelatedPlayerState = {nullptr};

};
