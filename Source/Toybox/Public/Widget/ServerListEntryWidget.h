// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ServerListEntryWidget.generated.h"

class UTextBlock;
/**
 * 
 */



UCLASS()
class TOYBOX_API UServerListEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	DECLARE_DELEGATE_OneParam(FOnServerSelected, const TWeakObjectPtr<UServerListEntryWidget>)
public:
	
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;
	
	bool IsSelectedEntry() const;

	int32 GetCachedIndex() const;

	void SetServerListEntry(int32 ServerIndex, const FText& ServerName);

	void Reset();
	
	FOnServerSelected OnServerSelected;
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectButton = {nullptr};

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Index = {nullptr};
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ServerName = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	FSlateFontInfo NumberFont;

	UFUNCTION()
	void OnSelectButtonClicked();

private:
	FLinearColor NormalBackgroundColor;

	bool bIsSelected = false;

	int32 CachedIndex = 0;
};
