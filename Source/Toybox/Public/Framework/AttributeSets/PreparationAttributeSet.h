// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PreparationAttributeSet.generated.h"


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
class TOYBOX_API UPreparationAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Money)
	FGameplayAttributeData Money;
	ATTRIBUTE_ACCESSORS(ThisClass, Money)

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UFUNCTION()
	void OnRep_Money(const FGameplayAttributeData& OldValue) const;
};
