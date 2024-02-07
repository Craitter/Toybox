// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpec.h"
//#include "GameplayTagContainer.h"
#include "ToyboxGlobalAbilitySystem.generated.h"


struct FToyboxAbilitySet_GrantedHandles;
struct FActiveGameplayEffectHandle;
struct FGameplayAbilitySpecHandle;
class UToyboxAbilitySystemComponent;
class UGameplayEffect;
class UGameplayAbility;

USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()

	FGameplayTagContainer AbilityRequiredTags;
	FGameplayTagContainer AbilityIgnoredTags;
	
	UPROPERTY()
	TMap<TWeakObjectPtr<UToyboxAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	void AddToAbilitySystemComponent(TSubclassOf<UGameplayAbility> Ability, TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent);
	void RemoveFromAbilitySystemComponent(TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent);
	void RemoveFromAll(const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags);
};

USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()

	FGameplayTagContainer EffectRequiredTags;
	FGameplayTagContainer EffectIgnoredTags;
	
	UPROPERTY()
	TMap<TWeakObjectPtr<UToyboxAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	void AddToAbilitySystemComponent(const TSubclassOf<UGameplayEffect> Effect, const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent);
	void RemoveFromAbilitySystemComponent(TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent);
	//You can leave the containers empty, then it will happen without tags
	void RemoveFromAll(const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags);
};

USTRUCT()
struct FGlobalAppliedAttributeSetList
{
	GENERATED_BODY()

	FGameplayTagContainer AttributeRequiredTags;
	FGameplayTagContainer AttributeIgnoredTags;
	
	UPROPERTY()
	TMap<TWeakObjectPtr<UToyboxAbilitySystemComponent>, TWeakObjectPtr<UAttributeSet>> AttributeSets;

	void AddToAbilitySystemComponent(TSubclassOf<UAttributeSet> AttributeSet, TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent);
	void RemoveFromAbilitySystemComponent(TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent);
	//You can leave the containers empty, then it will happen without tags
	void RemoveFromAll(const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags);
};

/**
 * 
 */
UCLASS()
class TOYBOX_API UToyboxGlobalAbilitySystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UToyboxGlobalAbilitySystem();

	//You can leave the containers empty, then it will happen without tags,
	//Everything Granted with those functions wont consider Tags that have been added or removed after application
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Toybox")
	void ApplyAttributeSetToAll(const TSubclassOf<UAttributeSet> AttributeSet,
		const FGameplayTagContainer& RequiredTags,const FGameplayTagContainer& IgnoredTags);

	//You can leave the containers empty, then it will happen without tags,
	//Everything Granted with those functions wont consider Tags that have been added or removed after application
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Toybox")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability,
		const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags);

	//You can leave the containers empty, then it will happen without tags,
	//Everything Granted with those functions wont consider Tags that have been added or removed after application
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Toybox")
	void ApplyEffectToAll(const TSubclassOf<UGameplayEffect> Effect,
		const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags);

	//You can leave the containers empty, then it will happen without tags,
	//Everything Granted with those functions wont consider Tags that have been added or removed after application
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Toybox")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability,
		const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags);

	//You can leave the containers empty, then it will happen without tags,
	//Everything Granted with those functions wont consider Tags that have been added or removed after application
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Toybox")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect,
		const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags);

	//You can leave the containers empty, then it will happen without tags,
	//Everything Granted with those functions wont consider Tags that have been added or removed after application
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Toybox")
	void RemoveAttributeSetFromAll(const TSubclassOf<UAttributeSet> AttributeSet,
		const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags);
	

	/** Register an AbilitySystemComponent with global system and apply any active global effects/abilities. */
	void RegisterAbilitySystemComponent(TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent);

	/** Removes an AbilitySystemComponent from the global system, along with any active global effects/abilities. */
	void UnregisterAbilitySystemComponent(TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent);

private:
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	UPROPERTY()
	TMap<TSubclassOf<UAttributeSet>, FGlobalAppliedAttributeSetList> AppliedAttributes;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<UToyboxAbilitySystemComponent>> RegisteredAbilitySystemComponents;
};
