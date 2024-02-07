// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_BurstLatent.h"
#include "DamageTakenCue.generated.h"

enum EHitDirection
{
	Front,

	Left,

	Right,

	Back,
};

/**
 * 
 */
UCLASS()
class TOYBOX_API ADamageTakenCue : public AGameplayCueNotify_BurstLatent
{
	GENERATED_BODY()

	ADamageTakenCue();

	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	static EHitDirection GetHitDirection(const FVector& ImpactPoint, TWeakObjectPtr<AActor> HitActor);

	static TWeakObjectPtr<UAnimMontage> GetRandomAnimationFromList(const TArray<TObjectPtr<UAnimMontage>>& MontageList);

	TWeakObjectPtr<UAnimMontage> GetAnimationToPlay(const bool bIsAlive, const EHitDirection Direction) const;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TArray<TObjectPtr<UAnimMontage>> FrontHitReacts;
	
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TArray<TObjectPtr<UAnimMontage>> BackHitReacts;
	
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TArray<TObjectPtr<UAnimMontage>> RightHitReacts;
	
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TArray<TObjectPtr<UAnimMontage>> LeftHitReacts;
};

