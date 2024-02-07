// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/AbilitySystemActor.h"

#include "Framework/ToyboxAbilitySystemComponent.h"
#include "Framework/AttributeSets/HealthAttributeSet.h"
#include "HelperObjects/ToyboxAbilitySet.h"
#include "HelperObjects/ToyboxGameplayTags.h"


// Sets default values
AAbilitySystemActor::AAbilitySystemActor()
{
	AbilitySystemComponent = CreateDefaultSubobject<UToyboxAbilitySystemComponent>("ToyboxAbilitySystemComponent");
	if (!ensure(IsValid(AbilitySystemComponent))) return;
	
	AbilitySystemComponent->ReplicationMode = EGameplayEffectReplicationMode::Minimal;

	bReplicates = true;

	// Required as this actor is mainly used a child actor
	// Leaving this set to true will stop this from loading on the clients!
	bNetLoadOnClient = false;
}

void AAbilitySystemActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	GrantAbilitySet(ActorAbilitySet, AbilitySet_GrantedHandles);
	
	const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
	
	AbilitySystemComponent->RegisterGameplayTagEvent(NativeTags.State_Dead, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnDeadTagChanged);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	const UHealthAttributeSet* HealthAttributeSet = Cast<UHealthAttributeSet>(
		AbilitySystemComponent->GetAttributeSet(UHealthAttributeSet::StaticClass()));
	if (HealthAttributeSet == nullptr)
	{
		return;
	}

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		HealthAttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
}

// Called when the game starts or when spawned
void AAbilitySystemActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAbilitySystemActor::OnDeadTagChanged(FGameplayTag ChangedTag, const int32 NewCount)
{
	if (NewCount > 0)
	{
		K2_OnDeath();
	}
}

void AAbilitySystemActor::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	K2_OnHealthChanged(Data.NewValue);
}

void AAbilitySystemActor::GrantAbilitySet(const TWeakObjectPtr<UToyboxAbilitySet> NewSet,
                                          FToyboxAbilitySet_GrantedHandles& OutGrantedHandles) const
{
	if (!ensure(NewSet.IsValid())) return;
	NewSet->GiveToAbilitySystem(AbilitySystemComponent, &OutGrantedHandles);
}

UAbilitySystemComponent* AAbilitySystemActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
