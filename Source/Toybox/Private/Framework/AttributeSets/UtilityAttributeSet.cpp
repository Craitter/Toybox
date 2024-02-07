// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AttributeSets/UtilityAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UUtilityAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;
	
	if (Attribute == GetAbsoluteDamageBuffAttribute())
	{
		SetAbsoluteDamageBuff(FMath::Max(GetAbsoluteDamageBuff(), 0.0f));
	}
}

void UUtilityAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, AbsoluteDamageBuff, COND_None, REPNOTIFY_Always);
}

void UUtilityAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetAbsoluteDamageBuffAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UUtilityAttributeSet::OnRep_AbsoluteDamageBuff(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, AbsoluteDamageBuff, OldValue)

}
