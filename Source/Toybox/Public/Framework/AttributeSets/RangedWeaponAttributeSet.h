// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Framework/ToyboxAbilitySystemComponent.h"
#include "RangedWeaponAttributeSet.generated.h"

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
class TOYBOX_API UWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Damage", ReplicatedUsing = OnRep_Damage)
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(ThisClass, Damage)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_CurrentAmmo)
	FGameplayAttributeData CurrentAmmo;
	ATTRIBUTE_ACCESSORS(ThisClass, CurrentAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxAmmo)
	FGameplayAttributeData MaxAmmo;
	ATTRIBUTE_ACCESSORS(ThisClass, MaxAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxAccuracy)
	FGameplayAttributeData MaxAccuracy;
	ATTRIBUTE_ACCESSORS(ThisClass, MaxAccuracy)

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MinAccuracy)
	FGameplayAttributeData MinAccuracy;
	ATTRIBUTE_ACCESSORS(ThisClass, MinAccuracy)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_CurrentBulletSpread)
	FGameplayAttributeData CurrentBulletSpread;
	ATTRIBUTE_ACCESSORS(ThisClass, CurrentBulletSpread)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_MaxRange)
	FGameplayAttributeData MaxRange;
	ATTRIBUTE_ACCESSORS(ThisClass, MaxRange)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_Recoil)
	FGameplayAttributeData Recoil;
	ATTRIBUTE_ACCESSORS(ThisClass, Recoil)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_ModifierResetTime)
	FGameplayAttributeData ModifierResetTime;
	ATTRIBUTE_ACCESSORS(ThisClass, ModifierResetTime)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_WarmupTime)
	FGameplayAttributeData WarmupTime;
	ATTRIBUTE_ACCESSORS(ThisClass, WarmupTime)
	
	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_AmmoUsedPerShot)
	FGameplayAttributeData AmmoUsedPerShot;
	ATTRIBUTE_ACCESSORS(ThisClass, AmmoUsedPerShot)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_CooldownAfterShotOrBurst)
	FGameplayAttributeData CooldownAfterShotOrBurst;
	ATTRIBUTE_ACCESSORS(ThisClass, CooldownAfterShotOrBurst)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_ShotsPerBurst)
	FGameplayAttributeData ShotsPerBurst;
	ATTRIBUTE_ACCESSORS(ThisClass, ShotsPerBurst)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_DelayBetweenBurstShots)
	FGameplayAttributeData DelayBetweenBurstShots;
	ATTRIBUTE_ACCESSORS(ThisClass, DelayBetweenBurstShots)
	
	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_MaxBulletSpreadInDegrees)
	FGameplayAttributeData MaxBulletSpreadInDegrees;
	ATTRIBUTE_ACCESSORS(ThisClass, MaxBulletSpreadInDegrees)
	
	UPROPERTY(BlueprintReadOnly, Category = "Modifiers", ReplicatedUsing = OnRep_BulletWhenLeastAccurate)
	FGameplayAttributeData BulletWhenLeastAccurate;
	ATTRIBUTE_ACCESSORS(ThisClass, BulletWhenLeastAccurate)	

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
protected:
	UFUNCTION()
	void OnRep_Damage(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_CurrentAmmo(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_MaxAmmo(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_MaxAccuracy(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_MinAccuracy(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_CurrentBulletSpread(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_MaxRange(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_Recoil(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_ModifierResetTime(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_WarmupTime(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_AmmoUsedPerShot(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_CooldownAfterShotOrBurst(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_ShotsPerBurst(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_DelayBetweenBurstShots(const FGameplayAttributeData& OldValue) const;
	
	UFUNCTION()
	void OnRep_MaxBulletSpreadInDegrees(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_BulletWhenLeastAccurate(const FGameplayAttributeData& OldValue) const;
};
