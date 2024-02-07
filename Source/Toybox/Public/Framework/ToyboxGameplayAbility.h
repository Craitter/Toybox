// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Actors/ToyboxCharacter.h"
#include "Actors/ToyboxPlayerController.h"
#include "ToyboxGameplayAbility.generated.h"


class UToyboxAbilityCost;
/**
 * EToyboxAbilityActivationPolicy
 *
 *	Defines how an ability is meant to activate.
 */
UENUM(BlueprintType)
enum class EToyboxAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered.
	OnInputTriggered,

	// Continually try to activate the ability while the input is active.
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned.
	OnSpawn
};


/**
 * EToyboxAbilityActivationGroup
 *
 *	Defines how an ability activates in relation to other abilities.
 */
UENUM(BlueprintType)
enum class EToyboxAbilityActivationGroup : uint8
{
	// Ability runs independently of all other abilities.
	Independent,

	// Ability is canceled and replaced by other exclusive abilities.
	Exclusive_Replaceable,

	// Ability blocks all other exclusive abilities from activating.
	Exclusive_Blocking,

	MAX	UMETA(Hidden)
};

/**
 * 
 */
UCLASS(Abstract, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class TOYBOX_API UToyboxGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class UToyboxAbilitySystemComponent;

public:
	UToyboxGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Toybox|Ability")
	UToyboxAbilitySystemComponent* GetToyboxAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Toybox|Ability")
	AToyboxPlayerController* GetToyboxPlayerControllerFromActorInfo() const;
	

	UFUNCTION(BlueprintCallable, Category = "Toybox|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Toybox|Ability")
	AToyboxCharacter* GetToyboxCharacterFromActorInfo() const;
	
	EToyboxAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	EToyboxAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

	// Returns true if the requested activation group is a valid transition.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Toybox|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool CanChangeActivationGroup(EToyboxAbilityActivationGroup NewGroup) const;

	// Tries to change the activation group.  Returns true if it successfully changed.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Toybox|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ChangeActivationGroup(EToyboxAbilityActivationGroup NewGroup);	
	
	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	//~End of UGameplayAbility interface

	virtual void OnPawnAvatarSet();
	const FScalableFloat& GetCooldownDuration() const;
	
	/** Called when this ability is granted to the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

	/** Called when this ability is removed from the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	void K2_OnAbilityRemoved();

	/** Called when the ability system is initialized with a pawn avatar. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	void K2_OnPawnAvatarSet();

protected:
	// If true, extra information should be logged when this ability is canceled. This is temporary, used for tracking a bugs.
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	bool bLogCanceling;
	
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toybox|AbilityActivation")
	EToyboxAbilityActivationPolicy ActivationPolicy;

	// Defines the relationship between this ability activating and other abilities activating.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toybox|AbilityActivation")
	EToyboxAbilityActivationGroup ActivationGroup;
	
	// Additional costs that must be paid to activate this ability
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Costs")
	TArray<TObjectPtr<UToyboxAbilityCost>> AdditionalCosts;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
	FScalableFloat CooldownDuration;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
	FGameplayTagContainer CooldownTags;

private:
	AController* FindControllerInOwnerChain() const;

	void DoesAbilitySatisfyTargetTagRequirements(const FGameplayTagContainer* TargetTags, bool& bBlocked,
												 bool& bMissing) const;
	void DoesAbilitySatisfySourceTagRequirements(const FGameplayTagContainer* SourceTags, bool& bBlocked,
												 bool& bMissing) const;
	void DoesAbilitySatisfyActivationTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent,
													 bool& bBlocked,
													 bool& bMissing) const;
#if !UE_BUILD_SHIPPING
	void LogCancelledAbility() const;
#endif // !UE_BUILD_SHIPPING

};
