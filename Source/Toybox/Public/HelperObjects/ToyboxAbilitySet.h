// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ToyboxAbilitySet.generated.h"

/**
 * 
 */

class UAttributeSet;
class UGameplayEffect;
class UToyboxAbilitySystemComponent;
class UToyboxGameplayAbility;


/**
 * FToyboxAbilitySet_GameplayAbility
 *
 *	Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct FToyboxAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UToyboxGameplayAbility> Ability = {nullptr};

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1", UIMin = "1"))
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag = FGameplayTag::EmptyTag;
};


/**
 * FToyboxAbilitySet_GameplayEffect
 *
 *	Data used by the ability set to grant gameplay effects.
 */
USTRUCT(BlueprintType)
struct FToyboxAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = {nullptr};

	// Level of gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1", UIMin = "1.0"))
	float EffectLevel = 1.0f;
};

/**
 * FToyboxAbilitySet_AttributeSet
 *
 *	Data used by the ability set to grant attribute sets.
 */
USTRUCT(BlueprintType)
struct FToyboxAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet = {nullptr};
};


/**
 * FToyboxAbilitySet_GrantedHandles
 *
 *	Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct FToyboxAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(TObjectPtr<UAttributeSet> Set);

	void TakeFromAbilitySystem(const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent);

protected:
	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// Pointers to the granted attribute sets
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};


/**
 * UToyboxAbilitySet
 *
 *	Non-mutable data asset used to grant gameplay abilities and gameplay effects.
 */

UCLASS()
class TOYBOX_API UToyboxAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:
	UToyboxAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	void GiveAbilities(TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent,
	                   FToyboxAbilitySet_GrantedHandles* OutGrantedHandles, TObjectPtr<UObject> SourceObject) const;
	void GiveEffects(TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent,
	                 FToyboxAbilitySet_GrantedHandles* OutGrantedHandles) const;
	void GiveAttributeSets(TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent,
	                     FToyboxAbilitySet_GrantedHandles* OutGrantedHandles) const;

	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.
	void GiveToAbilitySystem(TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent, FToyboxAbilitySet_GrantedHandles* OutGrantedHandles, TObjectPtr<UObject> SourceObject = nullptr) const;

	//Sets Granted with those functions wont consider Tags that have been added or removed after application
	void GrantToAllAbilitySystems(TWeakObjectPtr<UWorld> World, const FGameplayTagContainer& RequiredTags = FGameplayTagContainer(), const FGameplayTagContainer& IgnoredTags = FGameplayTagContainer());
	void RemoveFromAllAbilitySystems(TWeakObjectPtr<UWorld> World, const FGameplayTagContainer& RequiredTags = FGameplayTagContainer(), const FGameplayTagContainer& IgnoredTags = FGameplayTagContainer());

protected:
	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FToyboxAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FToyboxAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// Attribute sets to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
	TArray<FToyboxAbilitySet_AttributeSet> GrantedAttributes;
};
