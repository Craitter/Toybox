// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/AttributeSets/RangedWeaponAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Toybox/Toybox.h"


void UWeaponAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UWeaponAttributeSet::PostGameplayEffectExecute)	

	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;
	
	if (Attribute == GetCurrentAmmoAttribute())
	{
		SetCurrentAmmo(FMath::Clamp(GetCurrentAmmo(), 0.0f, GetMaxAmmo()));
	}
	else if (Attribute == GetCooldownAfterShotOrBurstAttribute())
	{
		SetCooldownAfterShotOrBurst(FMath::Max(GetCooldownAfterShotOrBurst(), 0.01f));
	}
	else if (Attribute == GetDelayBetweenBurstShotsAttribute())
	{
		SetDelayBetweenBurstShots(FMath::Max(GetDelayBetweenBurstShots(), 0.01f));
	}
	else if (Attribute == GetMaxAccuracyAttribute())
	{
		SetMaxAccuracy(FMath::Clamp(GetMaxAccuracy(), 0.0f, 1.0f));
	}
	else if (Attribute == GetMinAccuracyAttribute())
	{
		SetMinAccuracy(FMath::Clamp(GetMinAccuracy(), 0.0f, 1.0f));
	}
	else if (Attribute == GetBulletWhenLeastAccurateAttribute())
	{
		SetBulletWhenLeastAccurate(FMath::Max(GetBulletWhenLeastAccurate(), 1.0f));
	}
	else if (Attribute == GetCurrentBulletSpreadAttribute())
	{
		const float MaxSpreadDegrees = GetMaxBulletSpreadInDegrees();
		const float MaxBulletSpread = MaxSpreadDegrees - (MaxSpreadDegrees * GetMinAccuracy());
		
		SetCurrentBulletSpread(FMath::Clamp(GetCurrentBulletSpread(), 0.0f, MaxBulletSpread));
	}
}

void UWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Damage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, CurrentAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxAccuracy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MinAccuracy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, CurrentBulletSpread, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxRange, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Recoil, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, ModifierResetTime, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, WarmupTime, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, AmmoUsedPerShot, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, CooldownAfterShotOrBurst, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, ShotsPerBurst, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, DelayBetweenBurstShots, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxBulletSpreadInDegrees, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, BulletWhenLeastAccurate, COND_None, REPNOTIFY_Always);
}

void UWeaponAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetCurrentBulletSpreadAttribute())
	{
		const float MaxSpreadDegrees = GetMaxBulletSpreadInDegrees();
		const float MaxBulletSpread = MaxSpreadDegrees - (MaxSpreadDegrees * GetMinAccuracy());
		
		NewValue = FMath::Clamp(NewValue, 0.0f, MaxBulletSpread);
	}
}

void UWeaponAttributeSet::OnRep_Damage(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Damage, OldValue)
}

void UWeaponAttributeSet::OnRep_CurrentAmmo(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, CurrentAmmo, OldValue)
}

void UWeaponAttributeSet::OnRep_MaxAmmo(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxAmmo, OldValue)
}

void UWeaponAttributeSet::OnRep_MaxAccuracy(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxAccuracy, OldValue)
}

void UWeaponAttributeSet::OnRep_MinAccuracy(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MinAccuracy, OldValue)
}

void UWeaponAttributeSet::OnRep_CurrentBulletSpread(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, CurrentBulletSpread, OldValue)
}

void UWeaponAttributeSet::OnRep_MaxRange(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxRange, OldValue)
}

void UWeaponAttributeSet::OnRep_Recoil(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Recoil, OldValue)
}

void UWeaponAttributeSet::OnRep_ModifierResetTime(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, ModifierResetTime, OldValue)
}

void UWeaponAttributeSet::OnRep_WarmupTime(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, WarmupTime, OldValue)
}

void UWeaponAttributeSet::OnRep_AmmoUsedPerShot(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, AmmoUsedPerShot, OldValue)
}

void UWeaponAttributeSet::OnRep_CooldownAfterShotOrBurst(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, CooldownAfterShotOrBurst, OldValue)
}

void UWeaponAttributeSet::OnRep_ShotsPerBurst(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, ShotsPerBurst, OldValue)
}

void UWeaponAttributeSet::OnRep_DelayBetweenBurstShots(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, DelayBetweenBurstShots, OldValue)
}

void UWeaponAttributeSet::OnRep_MaxBulletSpreadInDegrees(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxBulletSpreadInDegrees, OldValue)
}

void UWeaponAttributeSet::OnRep_BulletWhenLeastAccurate(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, BulletWhenLeastAccurate, OldValue)
}
