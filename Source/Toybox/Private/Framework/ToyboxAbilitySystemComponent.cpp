// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ToyboxAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueFunctionLibrary.h"
#include "Framework/ToyboxGlobalAbilitySystem.h"
#include "HelperObjects/ToyboxAbilityTagRelationshipMapping.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");

UToyboxAbilitySystemComponent::UToyboxAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UToyboxAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const TWeakObjectPtr<UToyboxGlobalAbilitySystem> GlobalAbilitySystem = UWorld::GetSubsystem<UToyboxGlobalAbilitySystem>(GetWorld());
	if (GlobalAbilitySystem.IsValid())
	{
		GlobalAbilitySystem->UnregisterAbilitySystemComponent(this);
	}
	Super::EndPlay(EndPlayReason);
}

void UToyboxAbilitySystemComponent::NotifyNonInstancedAbilitiesPawnAvatarSet(const FGameplayAbilitySpec& AbilitySpec)
{
	TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
	for (TWeakObjectPtr<UGameplayAbility> AbilityInstance : Instances)
	{
		const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbilityInstance = CastChecked<UToyboxGameplayAbility>(AbilityInstance);
		ToyboxAbilityInstance->OnPawnAvatarSet();
	}
}

void UToyboxAbilitySystemComponent::NotifyAbilitiesPawnAvatarSet()
{
	// Notify all abilities that a new pawn avatar has been set
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		const TWeakObjectPtr<UToyboxGameplayAbility> AbilityCDO = CastChecked<UToyboxGameplayAbility>(AbilitySpec.Ability);

		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			NotifyNonInstancedAbilitiesPawnAvatarSet(AbilitySpec);
		}
		else
		{
			AbilityCDO->OnPawnAvatarSet();
		}
	}
}

void UToyboxAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UToyboxAbilitySystemComponent::InitAbilityActorInfo)
	
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const TWeakObjectPtr<AActor> OldAvatar = ActorInfo->AvatarActor;

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (OldAvatar == InAvatarActor) return;
	
	if (Cast<APawn>(InAvatarActor))
	{
		SetReplicatedLooseGameplayTagCount(NativeTags.Pawn, 1);
		SetLooseGameplayTagCount(NativeTags.Pawn, 1);
		
		NotifyAbilitiesPawnAvatarSet();

		// Register with the global system once we actually have a pawn avatar. We wait until this time since some globally-applied effects may require an avatar.
		const TWeakObjectPtr<UToyboxGlobalAbilitySystem> GlobalAbilitySystem = UWorld::GetSubsystem<UToyboxGlobalAbilitySystem>(GetWorld());
		if (GlobalAbilitySystem.IsValid())
		{
			GlobalAbilitySystem->RegisterAbilitySystemComponent(this);
		}

		TryActivateAbilitiesOnSpawn();
	}
	else
	{
		SetReplicatedLooseGameplayTagCount(NativeTags.Pawn, 0);
		SetLooseGameplayTagCount(NativeTags.Pawn, 0);
	}
}

void UToyboxAbilitySystemComponent::CancelAbility(const bool bReplicateCancelAbility, const FGameplayAbilitySpec& AbilitySpec, const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbilityInstance) const
{
	if (ToyboxAbilityInstance->CanBeCanceled())
	{
		ToyboxAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), ToyboxAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
	}
	else
	{
		UE_LOG(LogToyboxAbilitySystem, Warning, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *ToyboxAbilityInstance->GetName());
	}
}

void UToyboxAbilitySystemComponent::TryCancelInstancedAbilityByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility, const FGameplayAbilitySpec& AbilitySpec) const
{
	// Cancel all the spawned instances, not the CDO.
	TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
	for (UGameplayAbility* AbilityInstance : Instances)
	{
		const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbilityInstance = CastChecked<UToyboxGameplayAbility>(AbilityInstance);

		if (ShouldCancelFunc(ToyboxAbilityInstance, AbilitySpec.Handle))
		{
			CancelAbility(bReplicateCancelAbility, AbilitySpec, ToyboxAbilityInstance);
		}
	}
}

void UToyboxAbilitySystemComponent::CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc,
                                                          const bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbilityCDO = CastChecked<UToyboxGameplayAbility>(AbilitySpec.Ability);
		if (ToyboxAbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			TryCancelInstancedAbilityByFunc(ShouldCancelFunc, bReplicateCancelAbility, AbilitySpec);
		}
		else
		{
			// Cancel the non-instanced ability CDO.
			if (ShouldCancelFunc(ToyboxAbilityCDO, AbilitySpec.Handle))
			{
				// Non-instanced abilities can always be canceled.
				if (!ensure(ToyboxAbilityCDO->CanBeCanceled())) continue;
				ToyboxAbilityCDO->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
			}
		}
	}
}

void UToyboxAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbility, FGameplayAbilitySpecHandle Handle)
	{
		if (!ToyboxAbility.IsValid()) return false;
		const EToyboxAbilityActivationPolicy ActivationPolicy = ToyboxAbility->GetActivationPolicy();
		return ActivationPolicy == EToyboxAbilityActivationPolicy::OnInputTriggered || ActivationPolicy == EToyboxAbilityActivationPolicy::WhileInputActive;
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UToyboxAbilitySystemComponent::OnAbilityInputTagTriggered(const FGameplayTag& InputTag, const bool bIsInputPressed)
{
	if (!InputTag.IsValid())
	{
		return;
	}
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (IsValid(AbilitySpec.Ability) && AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			if (bIsInputPressed)
			{
				AbilityInputTagPressed(AbilitySpec.Handle);
			}
			else
			{
				AbilityInputTagReleased(AbilitySpec.Handle);
			}
		}
	}
}

void UToyboxAbilitySystemComponent::AbilityInputTagPressed(const FGameplayAbilitySpecHandle& Handle)
{
	InputPressedSpecHandles.AddUnique(Handle);
	InputHeldSpecHandles.AddUnique(Handle);
}

void UToyboxAbilitySystemComponent::AbilityInputTagReleased(const FGameplayAbilitySpecHandle& Handle)
{
	InputReleasedSpecHandles.AddUnique(Handle);
	InputHeldSpecHandles.Remove(Handle);
}

void UToyboxAbilitySystemComponent::ProcessInputHeldAbilities(TArray<FGameplayAbilitySpecHandle>& AbilitiesToActivate)
{
	//
	// Process all abilities that activate when the input is held.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (IsValid(AbilitySpec->Ability) && !AbilitySpec->IsActive())
			{
				const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbilityCDO = CastChecked<UToyboxGameplayAbility>(AbilitySpec->Ability);

				if (ToyboxAbilityCDO->GetActivationPolicy() == EToyboxAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}
}

void UToyboxAbilitySystemComponent::ProcessInputPressedAbilities(TArray<FGameplayAbilitySpecHandle>& AbilitiesToActivate)
{
	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (IsValid(AbilitySpec->Ability))
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbilityCDO = CastChecked<UToyboxGameplayAbility>(AbilitySpec->Ability);

					if (ToyboxAbilityCDO->GetActivationPolicy() == EToyboxAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}
}

void UToyboxAbilitySystemComponent::ProcessInputReleasedAbilities()
{
	//
	// Process all abilities that had their input released this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (IsValid(AbilitySpec->Ability))
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}
}

void UToyboxAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	ProcessInputHeldAbilities(AbilitiesToActivate);

	ProcessInputPressedAbilities(AbilitiesToActivate);

	//
	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	ProcessInputReleasedAbilities();

	//
	// Clear the cached ability handles.
	//
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UToyboxAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

bool UToyboxAbilitySystemComponent::IsActivationGroupBlocked(EToyboxAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case EToyboxAbilityActivationGroup::Independent:
		// Independent abilities are never blocked.
		bBlocked = false;
		break;

	case EToyboxAbilityActivationGroup::Exclusive_Replaceable:
	case EToyboxAbilityActivationGroup::Exclusive_Blocking:
		// Exclusive abilities can activate if nothing is blocking.
		bBlocked = ActivationGroupCounts[static_cast<uint8>(EToyboxAbilityActivationGroup::Exclusive_Blocking)] > 0;
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), static_cast<uint8>(Group));
		break;
	}

	return bBlocked;
}

void UToyboxAbilitySystemComponent::AddAbilityToActivationGroup(EToyboxAbilityActivationGroup Group,
	TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbility)
{
	if (!ensure(ToyboxAbility.Get())) return;
	check(ActivationGroupCounts[static_cast<uint8>(Group)] < INT32_MAX);

	ActivationGroupCounts[static_cast<uint8>(Group)]++;

	constexpr bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case EToyboxAbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
		break;

	case EToyboxAbilityActivationGroup::Exclusive_Replaceable:
	case EToyboxAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(EToyboxAbilityActivationGroup::Exclusive_Replaceable, ToyboxAbility, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), static_cast<uint8>(Group));
		break;
	}
	constexpr uint8 Replaceable = static_cast<uint8>(EToyboxAbilityActivationGroup::Exclusive_Replaceable);
	constexpr uint8 Blocking = static_cast<uint8>(EToyboxAbilityActivationGroup::Exclusive_Blocking);
	const int32 ExclusiveCount = ActivationGroupCounts[Replaceable] + ActivationGroupCounts[Blocking];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogToyboxAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void UToyboxAbilitySystemComponent::RemoveAbilityFromActivationGroup(EToyboxAbilityActivationGroup Group,
	UToyboxGameplayAbility* ToyboxAbility)
{
	check(ToyboxAbility);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] > 0);

	ActivationGroupCounts[static_cast<uint8>(Group)]--;
}

void UToyboxAbilitySystemComponent::CancelActivationGroupAbilities(EToyboxAbilityActivationGroup Group,
	TWeakObjectPtr<UToyboxGameplayAbility> IgnoreToyboxAbility, const bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreToyboxAbility](const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbility, FGameplayAbilitySpecHandle Handle)
	{
		return ToyboxAbility->GetActivationGroup() == Group && ToyboxAbility != IgnoreToyboxAbility;
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UToyboxAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle,
                                                         const FGameplayAbilityActivationInfo& ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const
{
	const TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

void UToyboxAbilitySystemComponent::SetTagRelationshipMapping(const TWeakObjectPtr<UToyboxAbilityTagRelationshipMapping> NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

void UToyboxAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping.IsValid())
	{
		TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

void UToyboxAbilitySystemComponent::CreateGameplayCueFromHitResult(const TWeakObjectPtr<AActor> OwnerActor,
	const FGameplayTag& CueTag, const FHitResult& HitResult)
{
	TWeakObjectPtr<AActor> AbilityActor = OwnerActor;
	if (!AbilityActor.IsValid())
	{
		AbilityActor = HitResult.GetComponent()->GetOwner();
	}
	
	const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilityActor.Get());

	if (AbilitySystemComponent.IsValid())
	{	
		AbilitySystemComponent->ExecuteGameplayCue(CueTag, UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(HitResult));
	}
}


void UToyboxAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		const TWeakObjectPtr<UToyboxGameplayAbility> ToyboxAbilityCDO = CastChecked<UToyboxGameplayAbility>(AbilitySpec.Ability);
		ToyboxAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
	}
}

void UToyboxAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);
	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

void UToyboxAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

void UToyboxAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	const TObjectPtr<UToyboxGameplayAbility> ToyboxAbility = CastChecked<UToyboxGameplayAbility>(Ability);

	AddAbilityToActivationGroup(ToyboxAbility->GetActivationGroup(), ToyboxAbility);
}

void UToyboxAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
	bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);
	
	const TObjectPtr<UToyboxGameplayAbility> ToyboxAbility = CastChecked<UToyboxGameplayAbility>(Ability);

	RemoveAbilityFromActivationGroup(ToyboxAbility->GetActivationGroup(), ToyboxAbility);
}

void UToyboxAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags,
	UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags,
	bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping.IsValid())
	{
		// Use the mapping to expand the ability tags into block and cancel tag
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);

	//@TODO: Apply any special logic like blocking input or movement
}

void UToyboxAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags,
	UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: Apply any special logic like blocking input or movement
}


