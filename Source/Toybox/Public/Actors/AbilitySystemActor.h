// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "HelperObjects/ToyboxAbilitySet.h"
#include "AbilitySystemActor.generated.h"

struct FGameplayTag;
class UToyboxAbilitySet;
class UToyboxAbilitySystemComponent;
struct FOnAttributeChangeData;


UCLASS()
class TOYBOX_API AAbilitySystemActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAbilitySystemActor();

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "AbilitySet")
	TObjectPtr<UToyboxAbilitySet> ActorAbilitySet = {nullptr};

protected:
	FToyboxAbilitySet_GrantedHandles AbilitySet_GrantedHandles;

	virtual void PostInitializeComponents() override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnDeadTagChanged(FGameplayTag ChangedTag, const int32 NewCount);

	UFUNCTION(BlueprintImplementableEvent)
	void K2_OnDeath();

	UFUNCTION(BlueprintImplementableEvent)
	void K2_OnHealthChanged(float NewHealth);

	void OnHealthChanged(const FOnAttributeChangeData& Data);
	
	void GrantAbilitySet(const TWeakObjectPtr<UToyboxAbilitySet> NewSet, FToyboxAbilitySet_GrantedHandles& OutGrantedHandles) const;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};
