// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "ToyboxInputConfig.generated.h"

class UInputAction;

/**
 * FTaggedInputAction
 *
 * Struct used to map an input action to a gameplay input tag.
 */
USTRUCT(BlueprintType)
struct FTaggedInputAction
{
	GENERATED_BODY()

public:
	//This has to be a const UInputAction, this changes a lot!
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const UInputAction> InputAction = {nullptr};

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag = FGameplayTag::EmptyTag;
};


UCLASS()
class TOYBOX_API UToyboxInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// Returns the first Native Input Action associated with a given tag.
	TWeakObjectPtr<const UInputAction> FindNativeInputActionForTag(const FGameplayTag& InputTag,
	                                                               bool bLogNotFound) const;
	// Returns the first Ability Input Action associated with a given tag.
	TWeakObjectPtr<const UInputAction> FindAbilityInputActionForTag(const FGameplayTag& InputTag,
	                                                                bool bLogNotFound = true) const;

	// List of input actions used by the owner. These input actions are mapped to a gameplay tag and must be manually bound.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FTaggedInputAction> NativeInputActions;

	// List of input actions used by the owner.  These input actions are mapped to a gameplay tag and are automatically
	// bound to abilities with matching input tags.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FTaggedInputAction> AbilityInputActions;
};
