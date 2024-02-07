// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoWidget.generated.h"

class UToyboxCharacterData;
struct FOnAttributeChangeData;
class UTextBlock;
/**
 * 
 */
UCLASS()
class TOYBOX_API UAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCharacterDataChanged(TWeakObjectPtr<UToyboxCharacterData> NewCharacterData);
	void OnAmmoChanged(const FOnAttributeChangeData& Data) const;
	void SetCurrentAmmo(float NewAmmo) const;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BulletCount = {nullptr};
	
	bool bWasDestroyed = false;
};
