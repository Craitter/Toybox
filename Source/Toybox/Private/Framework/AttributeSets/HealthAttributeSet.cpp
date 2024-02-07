// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AttributeSets/HealthAttributeSet.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UHealthAttributeSet::PostGameplayEffectExecute)

	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	
	if (Data.EvaluatedData.Attribute == GetDamageHealthAttribute() || Data.EvaluatedData.Attribute == GetDamageShieldAttribute())
	{
		DealDamage(Data.Target);
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Handle other health changes.
		// Health loss should go through Damage.
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	} // Health
	else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		// Handle shield changes.
		SetShield(FMath::Clamp(GetShield(), 0.0f, GetMaxShield()));
	} // Mana
	else if (Data.EvaluatedData.Attribute == GetArmorAttribute())
	{
		// Handle Damage Resistance changes.
		SetArmor(FMath::Clamp(GetArmor(), MinArmor, MaxArmor));
	}
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, HealthRegenPerSecond, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, ShieldRegenPerSecond, COND_None, REPNOTIFY_Always);
}

void UHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Health, OldHealth)
}

void UHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxHealth, OldMaxHealth)
}

void UHealthAttributeSet::OnRep_HealthRegenPerSecond(const FGameplayAttributeData& OldHealthRegenPerSecond) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, HealthRegenPerSecond, OldHealthRegenPerSecond)
}

void UHealthAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldShield) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Shield, OldShield)
}

void UHealthAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxShield, OldMaxShield)
}

void UHealthAttributeSet::OnRep_ShieldRegenPerSecond(const FGameplayAttributeData& OldShieldRegenPerSecond) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Armor, OldShieldRegenPerSecond)
}

void UHealthAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Armor, OldArmor)
}

void UHealthAttributeSet::DealDamage(UAbilitySystemComponent& Target)
{
	const float LocalDamageShieldDone = GetDamageShield();
	SetDamageShield(0.0f);
	
	if (LocalDamageShieldDone > UE_FLOAT_NORMAL_THRESH)
	{
		const float ShieldAfterDamage = GetShield() - LocalDamageShieldDone;
		SetShield(FMath::Clamp(ShieldAfterDamage, 0.0f, GetMaxShield()));
	}
	
	const float LocalDamageHealthDone = GetDamageHealth();
	SetDamageHealth(0.0f);
	
	if (LocalDamageHealthDone > UE_FLOAT_NORMAL_THRESH)
	{
		const float NewHealth = GetHealth() - LocalDamageHealthDone;
		SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

		if (NewHealth < UE_FLOAT_NORMAL_THRESH)
		{
			const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
			
			Target.AddLooseGameplayTag(NativeTags.State_Dead);
			Target.AddReplicatedLooseGameplayTag(NativeTags.State_Dead);

			const TWeakObjectPtr<AToyboxCharacter> Character = Cast<AToyboxCharacter>(Target.GetAvatarActor());
			if (Character.IsValid())
			{
				// ReSharper disable once CppExpressionWithoutSideEffects
				Character->OnDeath.ExecuteIfBound();
			}
		}
	}
}
