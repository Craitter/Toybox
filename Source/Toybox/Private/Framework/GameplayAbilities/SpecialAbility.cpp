// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameplayAbilities/SpecialAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "Toybox/Toybox.h"


USpecialAbility::USpecialAbility()
{
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	AttackerTag = NativeTags.Team_Attacker;
	DefenderTag = NativeTags.Team_Defender;

	AbilityTags.AddTag(NativeTags.AbilityTag_Combat_Special);
	bServerRespectsRemoteAbilityCancellation = false;
}

bool USpecialAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                         const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (ActorInfo == nullptr || !ActorInfo->AbilitySystemComponent.IsValid() || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void USpecialAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Character = Cast<AToyboxCharacter>(ActorInfo->AvatarActor);
	if (!ensure(Character.IsValid()))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}
	MontageToPlay = Character->GetSpecialAbilityMontage();

	if (!MontageToPlay.IsValid())
	{
		OnMontageFinished();
	}
	else
	{
		SpecialAbilityMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("SpecialAbilityMontage"), MontageToPlay.Get());
		if (!IsValid(SpecialAbilityMontageTask))
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		SpecialAbilityMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		SpecialAbilityMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
		SpecialAbilityMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
		SpecialAbilityMontageTask->ReadyForActivation();
	}
}

void USpecialAbility::ExecuteDefender(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	K2_ExecuteDefender();
}

void USpecialAbility::ExecuteAttacker(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	K2_ExecuteAttacker();
}

void USpecialAbility::GetAllAbilitySystemsOfTeamInRadius(const FGameplayTag& Team, const float Radius,
	const bool bOnlyEffectPawns, TArray<TWeakObjectPtr<UAbilitySystemComponent>>& OutAbilitySystems,
	const bool bOnlyUnblockedResults, const float RangeThatIsNeverBlocked) const
{
	TArray<FOverlapResult> Results;
	PerformOverlap(Radius, Results);

	for (const FOverlapResult& Result : Results)
	{
		if (Result.GetActor() == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("SpecialAbility tried to get all actors but an overlap result had no valid actor"));
			continue;
		}
		
		if (bOnlyEffectPawns && !Result.GetActor()->IsA<APawn>())
		{
			continue;
		}

		if (bOnlyUnblockedResults && !IsUnblocked(Result.GetComponent(), RangeThatIsNeverBlocked))
		{
			continue;
		}
		
		TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = TryGetAbilitySystemComponent(Result.GetActor());
		if (AbilitySystemComponent.IsValid() && IsInTeam(Team, AbilitySystemComponent))
		{
#if ENABLE_DRAW_DEBUG
			if (bDrawDebug)
			{
				DebugOverlap(20.0f, Result.GetActor()->GetActorLocation(), FColor::Red);
			}
#endif
			OutAbilitySystems.AddUnique(AbilitySystemComponent);
		}
	}
#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DebugOverlap(RangeThatIsNeverBlocked, Character->GetActorLocation(), FColor::Green);
	}
#endif
}

FGameplayEffectSpecHandle USpecialAbility::GetEffectHandle(const TSubclassOf<UGameplayEffect> Effect) const
{
	const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent;
	
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect,
		GetAbilityLevel(), AbilitySystemComponent->MakeEffectContext());
	
	return SpecHandle;
}

void USpecialAbility::ApplyEffectToTarget(const FGameplayEffectSpec* Spec,
	const TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystem)
{
	if (Spec != nullptr)
	{
		TargetAbilitySystem->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void USpecialAbility::ApplyEffectToTargets(const FGameplayEffectSpec* Spec,
	TArray<TWeakObjectPtr<UAbilitySystemComponent>> TargetAbilitySystems)
{
	for (const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem : TargetAbilitySystems)
	{
		ApplyEffectToTarget(Spec, AbilitySystem);
	}
}

void USpecialAbility::ApplySetByCallerMagnitude(const FGameplayTag& Tag, const float Magnitude, FGameplayEffectSpec* Spec)
{
	if (Spec != nullptr)
	{
		Spec->SetSetByCallerMagnitude(Tag, Magnitude);
	}
}

void USpecialAbility::PerformOverlap(const float Radius, TArray<FOverlapResult>& OutResults) const
{
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	Params.bTraceComplex = false;
	Character->GetWorld()->OverlapMultiByProfile(
		OutResults,
		Character->GetActorLocation(),
		FQuat::Identity,
		PROFILE_OVERLAP_ALL,
		FCollisionShape::MakeSphere(Radius),
		Params);
#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DebugOverlap(Radius, Character->GetActorLocation(), FColor::Green);
	}
#endif
}

bool USpecialAbility::IsInTeam(const FGameplayTag& Team, const TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponentToCheck)
{
	return AbilitySystemComponentToCheck->HasMatchingGameplayTag(Team);
}

TWeakObjectPtr<UAbilitySystemComponent> USpecialAbility::TryGetAbilitySystemComponent(const TWeakObjectPtr<AActor> Actor)
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor.Get(), false);
}

bool USpecialAbility::IsUnblocked(const TWeakObjectPtr<UPrimitiveComponent> ComponentToCheck, const float AlwaysHitRange) const
{
	if (!ComponentToCheck.IsValid())
	{
		return false;
	}
	if (AlwaysHitRange > FVector::Distance(ComponentToCheck->GetComponentLocation(), Character->GetActorLocation()))
	{
		return true;
	}
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character.Get());
	QueryParams.AddIgnoredActor(ComponentToCheck->GetOwner());
	
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByProfile(HitResult,
		Character->GetActorLocation(),
		ComponentToCheck->GetComponentLocation(),
		BULLET_COLLISION_PROFILE,
		QueryParams);

	
#if ENABLE_DRAW_DEBUG
	DebugTrace(HitResult, Character->GetActorLocation(), ComponentToCheck->GetComponentLocation());
#endif

	return !HitResult.bBlockingHit;
}

#if ENABLE_DRAW_DEBUG
void USpecialAbility::DebugOverlap(const float Radius, const FVector& Location, const FColor& Color) const
{
	DrawDebugSphere(Character->GetWorld(), Location, Radius, 10.0f, Color, false, 5.0f, 0.0f, 0.0f);
}

void USpecialAbility::DebugTrace(const FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const
{
	if (Character->bEnableWeaponDebug)
	{
		FColor DebugColor = FColor::Green;
		FVector DebugTraceStart = TraceStart;

		if (HitResult.bBlockingHit)
		{
			DrawDebugLine(
				GetWorld(),
				TraceStart,
				HitResult.ImpactPoint,
				DebugColor,
				false, 2.0f, 0,
				1.0f
			);

			DebugTraceStart = HitResult.ImpactPoint;
			DebugColor = FColor::Red;
		}

		DrawDebugLine(
			GetWorld(),
			DebugTraceStart,
			TraceEnd,
			DebugColor,
			false, 2.0f, 0,
			1.0f
		);
	}
}
#endif

void USpecialAbility::OnMontageFinished()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(USpecialAbility::OnMontageFinished)
	
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	if (CurrentActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(NativeTags.Team_Attacker))
	{
		ExecuteAttacker(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, &CurrentEventData);
	}
	else if (CurrentActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(NativeTags.Team_Defender))
	{
		ExecuteDefender(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, &CurrentEventData);
	}
	else if (HasAuthority(&CurrentActivationInfo))
	{
		checkf(false, TEXT("SpecialAbility Was Called without a Team Tag, this is always happening when PIE without going through lobby"));
	}
}

void USpecialAbility::OnMontageCancelled()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

