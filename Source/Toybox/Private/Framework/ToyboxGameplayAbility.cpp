// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/ToyboxGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "Framework/ToyboxAbilityCost.h"
#include "Framework/ToyboxAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"

//copied from Lyra, LyraGameplayAbility.cpp l.:26
#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
	if (!ensure(IsInstantiated()))																														\
	{																																					\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
		return ReturnValue;																																\
	}																																					\
}

UToyboxGameplayAbility::UToyboxGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = EToyboxAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EToyboxAbilityActivationGroup::Independent;

	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	ActivationBlockedTags.AddTag(NativeTags.State_Dead);

	bLogCanceling = false;
}

UToyboxAbilitySystemComponent* UToyboxGameplayAbility::GetToyboxAbilitySystemComponentFromActorInfo() const
{
	return CurrentActorInfo ? Cast<UToyboxAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr;
}

AToyboxPlayerController* UToyboxGameplayAbility::GetToyboxPlayerControllerFromActorInfo() const
{
	return CurrentActorInfo ? Cast<AToyboxPlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr;
}

AController* UToyboxGameplayAbility::FindControllerInOwnerChain() const
{
	// Look for a player controller or pawn in the owner chain.
	TWeakObjectPtr<AActor> TestActor = CurrentActorInfo->OwnerActor;
	while (TestActor.IsValid())
	{
		const TWeakObjectPtr<AController> Controller = Cast<AController>(TestActor);
		if (Controller.IsValid())
		{
			return Controller.Get();
		}

		const TWeakObjectPtr<APawn> Pawn = Cast<APawn>(TestActor);
		if (Pawn.IsValid())
		{
			return Pawn->GetController();
		}

		TestActor = TestActor->GetOwner();
	}
	return nullptr;
}

AController* UToyboxGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo != nullptr)
	{
		const TWeakObjectPtr<APlayerController> PlayerController = CurrentActorInfo->PlayerController;
		if (PlayerController.IsValid())
		{
			return PlayerController.Get();
		}
		
		return FindControllerInOwnerChain();
	}

	return nullptr;
}

AToyboxCharacter* UToyboxGameplayAbility::GetToyboxCharacterFromActorInfo() const
{
	return CurrentActorInfo ? Cast<AToyboxCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

void UToyboxGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec) const
{
	const bool bIsPredicting = Spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting;

	// Try to activate if activation policy is on spawn.
	if (ActorInfo != nullptr && !Spec.IsActive() && !bIsPredicting && ActivationPolicy == EToyboxAbilityActivationPolicy::OnSpawn)
	{
		const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = ActorInfo->AbilitySystemComponent;
		const TWeakObjectPtr<AActor> AvatarActor = ActorInfo->AvatarActor;

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (AbilitySystemComponent.IsValid() && AvatarActor.IsValid() && !AvatarActor->GetTearOff() && AvatarActor->GetLifeSpan() <= 0.0f)
		{
			const bool bIsLocalExecution = NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted || NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly;
			const bool bIsServerExecution = NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly || NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated;

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				AbilitySystemComponent->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

bool UToyboxGameplayAbility::CanChangeActivationGroup(EToyboxAbilityActivationGroup NewGroup) const
{
	if (!IsInstantiated() || !IsActive())
	{
		return false;
	}

	if (ActivationGroup == NewGroup)
	{
		return true;
	}

	const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = GetToyboxAbilitySystemComponentFromActorInfo();
	check(AbilitySystemComponent.Get());

	if (ActivationGroup != EToyboxAbilityActivationGroup::Exclusive_Blocking && AbilitySystemComponent->IsActivationGroupBlocked(NewGroup))
	{
		// This ability can't change groups if it's blocked (unless it is the one doing the blocking).
		return false;
	}

	if (NewGroup == EToyboxAbilityActivationGroup::Exclusive_Replaceable && !CanBeCanceled())
	{
		// This ability can't become replaceable if it can't be canceled.
		return false;
	}	
	return true;
}

bool UToyboxGameplayAbility::ChangeActivationGroup(EToyboxAbilityActivationGroup NewGroup)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);

	if (!CanChangeActivationGroup(NewGroup))
	{
		return false;
	}

	if (ActivationGroup != NewGroup)
	{
		const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = GetToyboxAbilitySystemComponentFromActorInfo();
		check(AbilitySystemComponent.Get());

		AbilitySystemComponent->RemoveAbilityFromActivationGroup(ActivationGroup, this);
		AbilitySystemComponent->AddAbilityToActivationGroup(NewGroup, this);

		ActivationGroup = NewGroup;
	}

	return true;
}

bool UToyboxGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (ActorInfo == nullptr || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = GetToyboxAbilitySystemComponentFromActorInfo();

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (AbilitySystemComponent->IsActivationGroupBlocked(ActivationGroup))
	{
		return false;
	}

	return true;
}

void UToyboxGameplayAbility::SetCanBeCanceled(const bool bCanBeCanceled)
{
	// The ability can not block canceling if it's replaceable.
	if (!bCanBeCanceled && ActivationGroup == EToyboxAbilityActivationGroup::Exclusive_Replaceable)
	{
		UE_LOG(LogToyboxAbilitySystem, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetName());
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UToyboxGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UToyboxGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UToyboxGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (bHasBlueprintActivate)
	{
		// A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
		K2_ActivateAbility();
	}
	else if (bHasBlueprintActivateFromEvent)
	{
		if (TriggerEventData != nullptr)
		{
			// A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
			K2_ActivateAbilityFromEvent(*TriggerEventData);
		}
		else
		{
			UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s expects event data but none is being supplied. Use Activate Ability instead of Activate Ability From Event."), *GetName());
			constexpr bool bReplicateEndAbility = false;
			constexpr bool bWasCancelled = true;
			EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		}
	}
	else
	{
		// Do stuff... if you overwrite this function, you could just copy this whole block and exchange the log with your logic
		//You may not want to call super if you want to call super and a BP event at anytime
		//You have to call commit() at one point
		UE_LOG(LogToyboxAbilitySystem, Warning, TEXT("Ability %s Has no Blueprint Event and no overwrite in C++, nothing will happen"), *GetName());
	}
}
#if !UE_BUILD_SHIPPING
void UToyboxGameplayAbility::LogCancelledAbility() const
{
	if (IsInstantiated())
	{
		UE_LOG(LogToyboxAbilitySystem, Warning, TEXT("========  (%s) canceled with EndAbility (locally controlled? %i) ========"), *GetName(), IsLocallyControlled());
		const TWeakObjectPtr<APlayerState> PlayerState = Cast<APlayerState>(GetOwningActorFromActorInfo());
		if (PlayerState.IsValid())
		{
			UE_LOG(LogToyboxAbilitySystem, Log, TEXT("Player Name: %s"), *PlayerState->GetPlayerName());
		}	
	}
	else
	{
		UE_LOG(LogToyboxAbilitySystem, Warning, TEXT("========  (%s) canceled with EndAbility ========"), *GetNameSafe(this));
	}
}
#endif // !UE_BUILD_SHIPPING

void UToyboxGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        bool bReplicateEndAbility, bool bWasCancelled)
{
#if !UE_BUILD_SHIPPING
	if (bWasCancelled && bLogCanceling)
	{
		LogCancelledAbility();
		
		PrintScriptCallstack();

		constexpr SIZE_T StackTraceSize = 65535; //Max Stack Size, I guess
		ANSICHAR* StackTrace = static_cast<ANSICHAR*>(FMemory::SystemMalloc(StackTraceSize)); // Allocation that Memory for a String
		if (StackTrace != nullptr)
		{
			StackTrace[0] = 0;
			// Walk the stack and dump it to the allocated memory.
			FPlatformStackWalk::StackWalkAndDump(StackTrace, StackTraceSize, 1); //Invoke the Callstack and clear it I think
			UE_LOG(LogToyboxAbilitySystem, Log, TEXT("Call Stack:\n%hs"), ANSI_TO_TCHAR(StackTrace)); //Print it out
			FMemory::SystemFree(StackTrace); //freeing memory
		}
	}
#endif // !UE_BUILD_SHIPPING
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UToyboxGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}

	// Verify we can afford any additional costs
	for (TObjectPtr<UToyboxAbilityCost> AdditionalCost : AdditionalCosts)
	{
		if (IsValid(AdditionalCost))
		{
			if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, OptionalRelevantTags))
			{
				return false;
			}
		}
	}

	return true;
}

void UToyboxGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	if (AdditionalCosts.IsEmpty()) return;
	
	check(ActorInfo);

	// Used to determine if the ability actually hit a target (as some costs are only spent on successful attempts)
	auto DetermineIfAbilityHitTarget = [&]()
	{
		if (ActorInfo->IsNetAuthority())
		{
			const TWeakObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = Cast<UToyboxAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
			if (AbilitySystemComponent.IsValid())
			{
				FGameplayAbilityTargetDataHandle TargetData;
				AbilitySystemComponent->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
				for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
				{
					if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
					{
						return true;
					}
				}
			}
		}
		return false;
	};

	// Pay any additional costs
	bool bAbilityHitTarget = false;
	bool bHasDeterminedIfAbilityHitTarget = false;
	for (TObjectPtr<UToyboxAbilityCost> AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
		{
			if (AdditionalCost->ShouldOnlyApplyCostOnHit())
			{
				if (!bHasDeterminedIfAbilityHitTarget)
				{
					bAbilityHitTarget = DetermineIfAbilityHitTarget();
					bHasDeterminedIfAbilityHitTarget = true;
				}

				if (!bAbilityHitTarget)
				{
					continue;
				}
			}
			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
}

void UToyboxGameplayAbility::DoesAbilitySatisfyTargetTagRequirements(const FGameplayTagContainer* TargetTags, bool& bBlocked, bool& bMissing) const
{
	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}
}

void UToyboxGameplayAbility::DoesAbilitySatisfySourceTagRequirements(const FGameplayTagContainer* SourceTags, bool& bBlocked, bool& bMissing) const
{
	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}
}

void UToyboxGameplayAbility::DoesAbilitySatisfyActivationTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, bool& bBlocked, bool& bMissing) const
{
	const UToyboxAbilitySystemComponent* ToyboxAbilitySystemComponent = Cast<UToyboxAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// Expand our ability tags to add additional required/blocked tags
	if (IsValid(ToyboxAbilitySystemComponent))
	{
		ToyboxAbilitySystemComponent->GetAdditionalActivationTagRequirements(AbilityTags, AllRequiredTags, AllBlockedTags);
	}

	// Check to see the required/blocked tags for this ability
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;
		
		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}
}

bool UToyboxGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent,
                                                               const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                                               FGameplayTagContainer* OptionalRelevantTags) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UToyboxGameplayAbility::DoesAbilitySatisfyTagRequirements)
	
	// Specialized version to handle death exclusion and AbilityTags expansion via AbilitySystemComponent
	bool bBlocked = false;
	bool bMissing = false;

	const UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// Check if any of this ability's tags are currently blocked
	if (AbilitySystemComponent.AreAbilityTagsBlocked(AbilityTags))
	{
		bBlocked = true;
	}

	DoesAbilitySatisfyActivationTagRequirements(AbilitySystemComponent, bBlocked, bMissing);

	DoesAbilitySatisfySourceTagRequirements(SourceTags, bBlocked, bMissing);

	DoesAbilitySatisfyTargetTagRequirements(TargetTags, bBlocked, bMissing);

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}

const FGameplayTagContainer* UToyboxGameplayAbility::GetCooldownTags() const
{
	//This Implementation is a simplified version,
	//that does not care about the tags the Cooldown GE has,
	//for an advanced versions see https://github.com/tranek/GASDocumentation#concepts-ge-cooldown
	return &CooldownTags;
}

void UToyboxGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const TWeakObjectPtr<UGameplayEffect> CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE.IsValid())
	{
		if (IsInstantiated())
		{
			const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
			SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTags);
			//Will be removed after CooldownDuration seconds so we dont have to cache it
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
		else
		{
			Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
		}
	}
}

void UToyboxGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}

const FScalableFloat& UToyboxGameplayAbility::GetCooldownDuration() const
{
	return CooldownDuration;
}
