// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/TeamApplicationRequirement.h"

#include "AbilitySystemComponent.h"
#include "HelperObjects/ToyboxGameplayTags.h"

bool UTeamApplicationRequirement::CanApplyGameplayEffect_Implementation(const UGameplayEffect* GameplayEffect,
                                                                        const FGameplayEffectSpec& Spec, UAbilitySystemComponent* ASC) const
{
	const TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystem = Spec.GetEffectContext().GetInstigatorAbilitySystemComponent();
	if (!ensure(SourceAbilitySystem.IsValid())) return false;
	if (!ensure(ASC != nullptr)) return false;
	
	return SourceAbilitySystem == ASC || !IsFriendly(SourceAbilitySystem, ASC);
}

bool UTeamApplicationRequirement::IsFriendly(const TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent,
                                             const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent)
{
	return IsAttackers(SourceAbilitySystemComponent, TargetAbilitySystemComponent) || IsDefenders(SourceAbilitySystemComponent, TargetAbilitySystemComponent);
}


bool UTeamApplicationRequirement::IsDefenders(const TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent,
                                              const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent)
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	return SourceAbilitySystemComponent->HasMatchingGameplayTag(NativeTags.Team_Defender) && TargetAbilitySystemComponent->HasMatchingGameplayTag(NativeTags.Team_Defender);
}

bool UTeamApplicationRequirement::IsAttackers(const TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent,
                                              const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent)
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	return SourceAbilitySystemComponent->HasMatchingGameplayTag(NativeTags.Team_Attacker) && TargetAbilitySystemComponent->HasMatchingGameplayTag(NativeTags.Team_Attacker);
}
