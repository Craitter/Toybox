// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthWidget.generated.h"

class UImage;
class UToyboxCharacterData;
class AMatchPlayerState;
class AToyboxPlayerState;
struct FOnAttributeChangeData;
class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class TOYBOX_API UHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	void TryRemoveBindings() const;

	virtual void NativeDestruct() override;

	void SetRelatedPlayerState(TWeakObjectPtr<APlayerState> PlayerState);

	void InitializeHealthWidget();
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ShieldBar = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthPoints = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ShieldPoints = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerName = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerIcon = {nullptr};

	void OnHealthChanged(const FOnAttributeChangeData& Data) const;

	void OnShieldChanged(const FOnAttributeChangeData& Data) const;

	void SetCurrentHealth(float NewHealth) const;

	void SetCurrentShield(float NewShield) const;

	UFUNCTION()
	void OnCharacterDataChanged(TWeakObjectPtr<UToyboxCharacterData> NewCharacterData);

private:
	FDelegateHandle HealthChangedHandle;

	FDelegateHandle ShieldChangedHandle;

	FNumberFormattingOptions AttributeNumberFormat;

	float MaxHealth = 0.0f;

	float MaxShield = 0.0f;

	TWeakObjectPtr<AMatchPlayerState> RelatedPlayerState = {nullptr};

	bool bWasDestroyed = false;
};
