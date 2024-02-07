// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Framework/ToyboxAbilitySystemComponent.h"
#include "HealthAttributeSet.generated.h"

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
class TOYBOX_API UHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(ThisClass, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(ThisClass, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_HealthRegenPerSecond)
	FGameplayAttributeData HealthRegenPerSecond;
	ATTRIBUTE_ACCESSORS(ThisClass, HealthRegenPerSecond)

	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(ThisClass, Shield)

	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(ThisClass, MaxShield)

	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_ShieldRegenPerSecond)
	FGameplayAttributeData ShieldRegenPerSecond;
	ATTRIBUTE_ACCESSORS(ThisClass, ShieldRegenPerSecond)

	UPROPERTY(BlueprintReadOnly, Category = "Armor", ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(ThisClass, Armor)
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData DamageHealth;
	ATTRIBUTE_ACCESSORS(ThisClass, DamageHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData DamageShield;
	ATTRIBUTE_ACCESSORS(ThisClass, DamageShield)

	const float MinArmor = 0.0f;

	const float MaxArmor = 1.0f;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_HealthRegenPerSecond(const FGameplayAttributeData& OldHealthRegenPerSecond) const;

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldShield) const;

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield) const;

	UFUNCTION()
	void OnRep_ShieldRegenPerSecond(const FGameplayAttributeData& OldShieldRegenPerSecond) const;

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;

private:
	void DealDamage(UAbilitySystemComponent& Target);
};
