// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/ToyboxGameplayAbility.h"
#include "SpecialAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class TOYBOX_API USpecialAbility : public UToyboxGameplayAbility
{
	GENERATED_BODY()

public:
	USpecialAbility();
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	virtual void ExecuteDefender(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	virtual void ExecuteAttacker(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "ExecuteAttacker", meta=(ScriptName = "ExecuteAttacker"))
	void K2_ExecuteAttacker();

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "ExecuteDefender", meta=(ScriptName = "ExecuteDefender"))
	void K2_ExecuteDefender();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> AttackerEffect = {nullptr};
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DefenderEffect = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	bool bDrawDebug = false;
	
	TWeakObjectPtr<AToyboxCharacter> Character = {nullptr};

	TWeakObjectPtr<UAnimMontage> MontageToPlay = {nullptr};

	FGameplayTag AttackerTag = FGameplayTag::EmptyTag;

	FGameplayTag DefenderTag = FGameplayTag::EmptyTag;

	void GetAllAbilitySystemsOfTeamInRadius(const FGameplayTag& Team, const float Radius, const bool bOnlyEffectPawns, TArray<TWeakObjectPtr<UAbilitySystemComponent>>&
	                                        OutAbilitySystems, const bool bOnlyUnblockedResults = false, const float RangeThatIsNeverBlocked = 0.0f) const;

	FGameplayEffectSpecHandle GetEffectHandle(TSubclassOf<UGameplayEffect> Effect) const;

	static void ApplyEffectToTarget(const FGameplayEffectSpec* Spec, TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystem);

	void ApplyEffectToTargets(const FGameplayEffectSpec* Spec, TArray<TWeakObjectPtr<UAbilitySystemComponent>> TargetAbilitySystems);

	static void ApplySetByCallerMagnitude(const FGameplayTag& Tag, float Magnitude, FGameplayEffectSpec* Spec);
	
private:
	void PerformOverlap(float Radius, TArray<FOverlapResult>& OutResults) const;

	static bool IsInTeam(const FGameplayTag& Team, TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponentToCheck);

	static TWeakObjectPtr<UAbilitySystemComponent> TryGetAbilitySystemComponent(TWeakObjectPtr<AActor> Actor);

	bool IsUnblocked(const TWeakObjectPtr<UPrimitiveComponent> ComponentToCheck, const float AlwaysHitRange) const;

#if ENABLE_DRAW_DEBUG
	void DebugOverlap(const float Radius, const FVector& Location, const FColor& Color) const;


	void DebugTrace(const FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const;

#endif
	
	TObjectPtr<UAbilityTask_PlayMontageAndWait> SpecialAbilityMontageTask;

	UFUNCTION()
	void OnMontageFinished();

	UFUNCTION()
	void OnMontageCancelled();
};
