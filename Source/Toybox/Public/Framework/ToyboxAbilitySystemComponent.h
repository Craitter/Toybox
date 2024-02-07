// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "ToyboxGameplayAbility.h"
#include "ToyboxAbilitySystemComponent.generated.h"

class UToyboxAbilityTagRelationshipMapping;
/**
 * 
 */


TOYBOX_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

UCLASS()
class TOYBOX_API UToyboxAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	//https://github.com/tranek/GASDocumentation#73-abilitysystemcomponent-replication-mode
public:
	UToyboxAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	//~End of UActorComponent interface

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	

	typedef TFunctionRef<bool(const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility);

	void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

	void OnAbilityInputTagTriggered(const FGameplayTag& InputTag, bool bIsInputPressed);
	void AbilityInputTagPressed(const FGameplayAbilitySpecHandle& Handle);
	void AbilityInputTagReleased(const FGameplayAbilitySpecHandle& Handle);

	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();

	bool IsActivationGroupBlocked(EToyboxAbilityActivationGroup Group) const;
	void AddAbilityToActivationGroup(EToyboxAbilityActivationGroup Group, TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbility);
	void RemoveAbilityFromActivationGroup(EToyboxAbilityActivationGroup Group, UToyboxGameplayAbility* ToyboxAbility);
	void CancelActivationGroupAbilities(EToyboxAbilityActivationGroup Group, TWeakObjectPtr<UToyboxGameplayAbility> IgnoreToyboxAbility, bool
	                                    bReplicateCancelAbility);

	/** Gets the ability target data associated with the given ability handle and activation info */
	void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, const FGameplayAbilityActivationInfo& ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const;

	/** Sets the current tag relationship mapping, if null it will clear it out */
	void SetTagRelationshipMapping(TWeakObjectPtr<UToyboxAbilityTagRelationshipMapping> NewMapping);
	
	/** Looks at ability tags and gathers additional required and blocking tags */
	void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

	static void CreateGameplayCueFromHitResult(const TWeakObjectPtr<AActor> OwnerActor,
		const FGameplayTag& CueTag, const FHitResult& HitResult);
	
protected:
	void TryActivateAbilitiesOnSpawn();

	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
	virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

	// If set, this table is used to look up tag relationships for activate and cancel
	TWeakObjectPtr<UToyboxAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// Number of abilities running in each activation group.
	int32 ActivationGroupCounts[static_cast<uint8>(EToyboxAbilityActivationGroup::MAX)];

private:
	static void NotifyNonInstancedAbilitiesPawnAvatarSet(const FGameplayAbilitySpec& AbilitySpec);
	void NotifyAbilitiesPawnAvatarSet();

	void CancelAbility(bool bReplicateCancelAbility, const FGameplayAbilitySpec& AbilitySpec,
					   TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbilityInstance) const;
	void TryCancelInstancedAbilityByFunc(
		const TFunctionRef<bool(TWeakObjectPtr<UToyboxGameplayAbility>, FGameplayAbilitySpecHandle)>& ShouldCancelFunc,
		bool bReplicateCancelAbility, const FGameplayAbilitySpec& AbilitySpec) const;

	void ProcessInputHeldAbilities(TArray<FGameplayAbilitySpecHandle>& AbilitiesToActivate);
	void ProcessInputPressedAbilities(TArray<FGameplayAbilitySpecHandle>& AbilitiesToActivate);
	void ProcessInputReleasedAbilities();
};
