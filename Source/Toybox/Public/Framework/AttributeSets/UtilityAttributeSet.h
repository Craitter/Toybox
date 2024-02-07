// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "UtilityAttributeSet.generated.h"

// See line 405 in AttributeSet.h for more info
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
/**
 * 
 */
UCLASS()
class TOYBOX_API UUtilityAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Utility|Buff|Damage", ReplicatedUsing = OnRep_AbsoluteDamageBuff)
	FGameplayAttributeData AbsoluteDamageBuff;
	ATTRIBUTE_ACCESSORS(ThisClass, AbsoluteDamageBuff)
	
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

protected:
	UFUNCTION()
	void OnRep_AbsoluteDamageBuff(const FGameplayAttributeData& OldValue) const;
};
