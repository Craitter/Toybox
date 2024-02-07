// Fill out your copyright notice in the Description page of Project Settings.


#include "HelperObjects/ToyboxAbilitySet.h"

#include "Framework/ToyboxAbilitySystemComponent.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "Framework/ToyboxGlobalAbilitySystem.h"
#include "Toybox/Toybox.h"


void FToyboxAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FToyboxAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FToyboxAbilitySet_GrantedHandles::AddAttributeSet(const TObjectPtr<UAttributeSet> Set)
{
	if (IsValid(Set))
	{
		GrantedAttributeSets.Add(Set);
	}
}

void FToyboxAbilitySet_GrantedHandles::TakeFromAbilitySystem(const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FToyboxAbilitySet_GrantedHandles::TakeFromAbilitySystem)

	check(AbilitySystemComponent.Get());

	if (!AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		AbilitySystemComponent->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}


UToyboxAbilitySet::UToyboxAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UToyboxAbilitySet::GiveAbilities(TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent, FToyboxAbilitySet_GrantedHandles* OutGrantedHandles, TObjectPtr<UObject> SourceObject) const
{
	// Grant the gameplay abilities.
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FToyboxAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogToyboxAbilitySystem, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}

		TObjectPtr<UToyboxGameplayAbility> AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UToyboxGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}
}

void UToyboxAbilitySet::GiveEffects(TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent, FToyboxAbilitySet_GrantedHandles* OutGrantedHandles) const
{
	// Grant the gameplay effects.
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FToyboxAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOG(LogToyboxAbilitySystem, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
			continue;
		}

		const TObjectPtr<UGameplayEffect> GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, AbilitySystemComponent->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}
}

void UToyboxAbilitySet::GiveAttributeSets(TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent, FToyboxAbilitySet_GrantedHandles* OutGrantedHandles) const
{
	// Grant the attribute sets.
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FToyboxAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			UE_LOG(LogToyboxAbilitySystem, Error, TEXT("GrantedAttributes[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
			continue;
		}

		TObjectPtr<UAttributeSet> NewSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwner(), SetToGrant.AttributeSet);
		AbilitySystemComponent->AddAttributeSetSubobject(NewSet.Get());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}
}

void UToyboxAbilitySet::GiveToAbilitySystem(TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent,
                                            FToyboxAbilitySet_GrantedHandles* OutGrantedHandles, TObjectPtr<UObject> SourceObject) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UToyboxAbilitySet::GiveToAbilitySystem)

	check(AbilitySystemComponent);
	
	if (!AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}
	GiveAbilities(AbilitySystemComponent, OutGrantedHandles, SourceObject);
	GiveAttributeSets(AbilitySystemComponent, OutGrantedHandles);
	GiveEffects(AbilitySystemComponent, OutGrantedHandles);
}

void UToyboxAbilitySet::GrantToAllAbilitySystems(TWeakObjectPtr<UWorld> World, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	check(World.Get());
	const TWeakObjectPtr<UToyboxGlobalAbilitySystem> GlobalAbilitySystem = World->GetSubsystem<UToyboxGlobalAbilitySystem>();
	if (!ensure(GlobalAbilitySystem.IsValid())) return;

	for (const FToyboxAbilitySet_AttributeSet& AttributeSet : GrantedAttributes)
	{
		GlobalAbilitySystem->ApplyAttributeSetToAll(AttributeSet.AttributeSet, RequiredTags, IgnoredTags);
	}

	for (const FToyboxAbilitySet_GameplayEffect& GameplayEffect : GrantedGameplayEffects)
	{
		GlobalAbilitySystem->ApplyEffectToAll(GameplayEffect.GameplayEffect, RequiredTags, IgnoredTags);
	}

	for (const FToyboxAbilitySet_GameplayAbility& GameplayAbility : GrantedGameplayAbilities)
	{
		GlobalAbilitySystem->ApplyAbilityToAll(GameplayAbility.Ability, RequiredTags, IgnoredTags);
	}
}


void UToyboxAbilitySet::RemoveFromAllAbilitySystems(const TWeakObjectPtr<UWorld> World, const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& IgnoredTags)
{
	check(World.Get());
	const TWeakObjectPtr<UToyboxGlobalAbilitySystem> GlobalAbilitySystem = World->GetSubsystem<UToyboxGlobalAbilitySystem>();
	if (!ensure(GlobalAbilitySystem.IsValid())) return;

	for (const FToyboxAbilitySet_AttributeSet& AttributeSet : GrantedAttributes)
	{
		GlobalAbilitySystem->RemoveAttributeSetFromAll(AttributeSet.AttributeSet, RequiredTags, IgnoredTags);
	}

	for (const FToyboxAbilitySet_GameplayEffect& GameplayEffect : GrantedGameplayEffects)
	{
		GlobalAbilitySystem->RemoveEffectFromAll(GameplayEffect.GameplayEffect, RequiredTags, IgnoredTags);
	}

	for (const FToyboxAbilitySet_GameplayAbility& GameplayAbility : GrantedGameplayAbilities)
	{
		GlobalAbilitySystem->RemoveAbilityFromAll(GameplayAbility.Ability, RequiredTags, IgnoredTags);
	}
}
