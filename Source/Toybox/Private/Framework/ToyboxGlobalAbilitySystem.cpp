// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ToyboxGlobalAbilitySystem.h"

#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "Framework/ToyboxAbilitySystemComponent.h"


static bool SatisfiesTagRequirements(const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	return !AbilitySystemComponent->HasAnyMatchingGameplayTags(IgnoredTags) && AbilitySystemComponent->HasAllMatchingGameplayTags(RequiredTags);
}

void FGlobalAppliedAbilityList::AddToAbilitySystemComponent(const TSubclassOf<UGameplayAbility> Ability,
                                         const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent)
{
	if (!AbilitySystemComponent.IsValid()) return;

	if (!SatisfiesTagRequirements(AbilitySystemComponent, AbilityRequiredTags, AbilityIgnoredTags))
	{
		return;
	}
	
	if (Handles.Find(AbilitySystemComponent.Get()))
	{
		RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}

	const TWeakObjectPtr<UGameplayAbility> AbilityCDO = Ability->GetDefaultObject<UGameplayAbility>();
	const FGameplayAbilitySpec AbilitySpec(AbilityCDO.Get());
	const FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
	Handles.Add(AbilitySystemComponent.Get(), AbilitySpecHandle);
}
//not caring for tags here, since we only call it to clean up atm
void FGlobalAppliedAbilityList::RemoveFromAbilitySystemComponent(const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent)
{
	if (!AbilitySystemComponent.IsValid()) return;
	if (const FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(AbilitySystemComponent.Get()))
	{
		AbilitySystemComponent->ClearAbility(*SpecHandle);
		Handles.Remove(AbilitySystemComponent.Get());
	}
}

void FGlobalAppliedAbilityList::RemoveFromAll(const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	for (auto& KeyValuePair : Handles)
	{
		if (KeyValuePair.Key != nullptr && SatisfiesTagRequirements(KeyValuePair.Key, AbilityRequiredTags, AbilityIgnoredTags))
		{
			KeyValuePair.Key->ClearAbility(KeyValuePair.Value);
		}
	}
	Handles.Empty();
}

void FGlobalAppliedEffectList::AddToAbilitySystemComponent(const TSubclassOf<UGameplayEffect> Effect,
                                        const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent)
{
	if (!AbilitySystemComponent.IsValid()) return;

	if (!SatisfiesTagRequirements(AbilitySystemComponent, EffectRequiredTags, EffectIgnoredTags))
	{
		return;
	}
	
	if (Handles.Find(AbilitySystemComponent))
	{
		RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}

	const TWeakObjectPtr<UGameplayEffect> GameplayEffectCDO = Effect->GetDefaultObject<UGameplayEffect>();
	const FActiveGameplayEffectHandle GameplayEffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffectCDO.Get(), 1.0f, AbilitySystemComponent->MakeEffectContext());
	Handles.Add(AbilitySystemComponent, GameplayEffectHandle);
}

void FGlobalAppliedEffectList::RemoveFromAbilitySystemComponent(const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent)
{
	if (!AbilitySystemComponent.IsValid()) return;
	if (const FActiveGameplayEffectHandle* EffectHandle = Handles.Find(AbilitySystemComponent))
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(*EffectHandle);
		Handles.Remove(AbilitySystemComponent);
	}
}

void FGlobalAppliedEffectList::RemoveFromAll(const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	for (auto& KeyValuePair : Handles)
	{
		if (KeyValuePair.Key != nullptr && SatisfiesTagRequirements(KeyValuePair.Key, RequiredTags, IgnoredTags))
		{
			KeyValuePair.Key->RemoveActiveGameplayEffect(KeyValuePair.Value);
		}
	}
	Handles.Empty();
}

void FGlobalAppliedAttributeSetList::AddToAbilitySystemComponent(TSubclassOf<UAttributeSet> AttributeSet,
	TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent)
{
	if (!AbilitySystemComponent.IsValid()) return;

	if (!SatisfiesTagRequirements(AbilitySystemComponent, AttributeRequiredTags, AttributeIgnoredTags))
	{
		return;
	}
	
	if (AbilitySystemComponent->GetAttributeSet(AttributeSet) != nullptr)
	{
		RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}

	const TObjectPtr<UAttributeSet> NewSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwner(), AttributeSet);
	AbilitySystemComponent->AddAttributeSetSubobject(NewSet.Get());
	AttributeSets.Add(AbilitySystemComponent, NewSet);
}

void FGlobalAppliedAttributeSetList::RemoveFromAbilitySystemComponent(
	TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent)
{
	if (!AbilitySystemComponent.IsValid()) return;
	
	if (const TWeakObjectPtr<UAttributeSet>* EffectHandle = AttributeSets.Find(AbilitySystemComponent))
	{
		AbilitySystemComponent->RemoveSpawnedAttribute(EffectHandle->Get());
		AttributeSets.Remove(AbilitySystemComponent);
	}
}

void FGlobalAppliedAttributeSetList::RemoveFromAll(const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	for (auto& KeyValuePair : AttributeSets)
	{
		if (KeyValuePair.Key != nullptr && SatisfiesTagRequirements(KeyValuePair.Key, RequiredTags, IgnoredTags))
		{
			KeyValuePair.Key->RemoveSpawnedAttribute(KeyValuePair.Value.Get());
		}
	}
	AttributeSets.Empty();
}

UToyboxGlobalAbilitySystem::UToyboxGlobalAbilitySystem()
{
}

void UToyboxGlobalAbilitySystem::ApplyAttributeSetToAll(const TSubclassOf<UAttributeSet> AttributeSet, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	if (AttributeSet.Get() != nullptr && !AppliedAttributes.Contains(AttributeSet))
	{
		FGlobalAppliedAttributeSetList& AttributeSetList = AppliedAttributes.Add(AttributeSet);		
		for (const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent : RegisteredAbilitySystemComponents)
		{
			AttributeSetList.AttributeRequiredTags = RequiredTags;
			AttributeSetList.AttributeIgnoredTags = IgnoredTags;
			AttributeSetList.AddToAbilitySystemComponent(AttributeSet, AbilitySystemComponent);
		}
	}
}

void UToyboxGlobalAbilitySystem::ApplyAbilityToAll(const TSubclassOf<UGameplayAbility> Ability, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	if (Ability.Get() != nullptr && !AppliedAbilities.Contains(Ability))
	{
		FGlobalAppliedAbilityList& AppliedAbilityList = AppliedAbilities.Add(Ability);		
		for (const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent : RegisteredAbilitySystemComponents)
		{
			AppliedAbilityList.AbilityRequiredTags = RequiredTags;
			AppliedAbilityList.AbilityIgnoredTags = IgnoredTags;
			AppliedAbilityList.AddToAbilitySystemComponent(Ability, AbilitySystemComponent);
		}
	}
}

void UToyboxGlobalAbilitySystem::ApplyEffectToAll(const TSubclassOf<UGameplayEffect> Effect, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	if (Effect.Get() != nullptr && !AppliedEffects.Contains(Effect))
	{
		FGlobalAppliedEffectList& AppliedEffectList = AppliedEffects.Add(Effect);
		for (const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent : RegisteredAbilitySystemComponents)
		{
			AppliedEffectList.EffectRequiredTags = RequiredTags;
			AppliedEffectList.EffectIgnoredTags = IgnoredTags;
			AppliedEffectList.AddToAbilitySystemComponent(Effect, AbilitySystemComponent);
		}
	}
}

void UToyboxGlobalAbilitySystem::RemoveAbilityFromAll(const TSubclassOf<UGameplayAbility> Ability, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	if (Ability.Get() != nullptr && AppliedAbilities.Contains(Ability))
	{
		FGlobalAppliedAbilityList& AppliedAbilityList = AppliedAbilities[Ability];
		AppliedAbilityList.RemoveFromAll(RequiredTags, IgnoredTags);
		AppliedAbilities.Remove(Ability);
	}
}

void UToyboxGlobalAbilitySystem::RemoveEffectFromAll(const TSubclassOf<UGameplayEffect> Effect, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	if (Effect.Get() != nullptr && AppliedEffects.Contains(Effect))
	{
		FGlobalAppliedEffectList& Entry = AppliedEffects[Effect];
		Entry.RemoveFromAll(RequiredTags, IgnoredTags);
		AppliedEffects.Remove(Effect);
	}
}

void UToyboxGlobalAbilitySystem::RemoveAttributeSetFromAll(const TSubclassOf<UAttributeSet> AttributeSet, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	if (AttributeSet.Get() != nullptr && AppliedAttributes.Contains(AttributeSet))
	{
		FGlobalAppliedAttributeSetList& Entry = AppliedAttributes[AttributeSet];
		Entry.RemoveFromAll(RequiredTags, IgnoredTags);
		AppliedAttributes.Remove(AttributeSet);
	}
}

void UToyboxGlobalAbilitySystem::RegisterAbilitySystemComponent(const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent)
{
	check(AbilitySystemComponent.Get());

	for (auto& KeyValuePair : AppliedAbilities)
	{
		KeyValuePair.Value.AddToAbilitySystemComponent(KeyValuePair.Key, AbilitySystemComponent);
	}
	for (auto& KeyValuePair : AppliedEffects)
	{
		KeyValuePair.Value.AddToAbilitySystemComponent(KeyValuePair.Key, AbilitySystemComponent);
	}

	for (auto& KeyValuePair : AppliedAttributes)
	{
		KeyValuePair.Value.AddToAbilitySystemComponent(KeyValuePair.Key, AbilitySystemComponent);
	}

	RegisteredAbilitySystemComponents.AddUnique(AbilitySystemComponent);
}

void UToyboxGlobalAbilitySystem::UnregisterAbilitySystemComponent(const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent)
{
	check(AbilitySystemComponent.Get());
	for (auto& KeyValuePair : AppliedAbilities)
	{
		KeyValuePair.Value.RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}
	for (auto& KeyValuePair : AppliedEffects)
	{
		KeyValuePair.Value.RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}
	for (auto& KeyValuePair : AppliedAttributes)
	{
		KeyValuePair.Value.RemoveFromAbilitySystemComponent(AbilitySystemComponent);
	}

	RegisteredAbilitySystemComponents.Remove(AbilitySystemComponent);
}
