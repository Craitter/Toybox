// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AttributeSets/PreparationAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UPreparationAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	
	if (Data.EvaluatedData.Attribute == GetMoneyAttribute())
	{
		SetMoney(FMath::Max(0.0f, GetMoney()));
	}
}

void UPreparationAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Money, COND_None, REPNOTIFY_Always);
}

void UPreparationAttributeSet::OnRep_Money(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Money, OldValue)
}
