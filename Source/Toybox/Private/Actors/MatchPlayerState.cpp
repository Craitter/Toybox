// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MatchPlayerState.h"

#include "Actors/MatchGameMode.h"
#include "Actors/MatchGameState.h"
#include "Actors/ToyboxGameState.h"
#include "Framework/ToyboxAbilitySystemComponent.h"
#include "Framework/AttributeSets/HealthAttributeSet.h"
#include "HelperObjects/ToyboxCharacterData.h"
#include "HelperObjects/ToyboxAbilitySet.h"
#include "HelperObjects/ToyboxAbilityTagRelationshipMapping.h"
#include "HelperObjects/ToyboxGameplayTags.h"
#include "HelperObjects/ToyboxInputConfig.h"
#include "Net/UnrealNetwork.h"
#include "Toybox/Toybox.h"

AMatchPlayerState::AMatchPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UToyboxAbilitySystemComponent>("ToyboxAbilitySystemComponent");
	if (!ensure(IsValid(AbilitySystemComponent))) return;
	
	AbilitySystemComponent->ReplicationMode = EGameplayEffectReplicationMode::Mixed;
	NetUpdateFrequency = 100.0f;
}

void AMatchPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	
	SetSelectedChampion(DefaultChampion);
	SetCharacterData(ChampionData->RelatedCharacterData.LoadSynchronous());
	
	OnPawnSet.AddDynamic(this, &ThisClass::OnNewPawnSet);
}

void AMatchPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GetWorld()->GameStateSetEvent.AddUObject(this, &ThisClass::OnGameStateSet);
	OnGameStateSet(GetWorld()->GetGameState());
}

void AMatchPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CharacterData);
}

void AMatchPlayerState::SetTeam(const FGameplayTag& NewTeamTag)
{
	if (NewTeamTag == CurrentTeamTag)
	{
		return;
	}
		
	if (CurrentTeamTag != FGameplayTag::EmptyTag)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentTeamTag);
		AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(CurrentTeamTag);
	}


	CurrentTeamTag = NewTeamTag;


	if (CurrentTeamTag != FGameplayTag::EmptyTag)
	{
		AbilitySystemComponent->AddLooseGameplayTag(CurrentTeamTag);
		AbilitySystemComponent->AddReplicatedLooseGameplayTag(CurrentTeamTag);
	}
		
	OnTeamChanged();
}

void AMatchPlayerState::OnTeamChanged()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AMatchPlayerState::OnTeamChanged)

	
	const FToyboxGameplayTags NativeTags = FToyboxGameplayTags::Get();
	
	if (CurrentTeamTag == NativeTags.Team_Attacker)
	{
		HandleJoinAttackerTeam();
	}
	else if (CurrentTeamTag == NativeTags.Team_Defender)
	{
		HandleJoinDefenderTeam();
	}
	else if (CurrentTeamTag == NativeTags.Team_Spectator)
	{
		HandleJoinSpectatorTeam();
	}
}

void AMatchPlayerState::OnGameStateSet(AGameStateBase* NewGameState)
{
	MatchGameState = Cast<AMatchGameState>(NewGameState);
}

void AMatchPlayerState::NotifyGameStatePawnDied() const
{
	if (!HasAuthority())
	{
		return;
	}
	if (!MatchGameState.IsValid())
	{
		return;
	}
	if (IsDefenderTeam())
	{
		MatchGameState->DefenderDied(GetPlayerController());			
	}
	else if (IsAttackerTeam())
	{
		MatchGameState->AttackerDied(GetPlayerController(), DeathCount);		
	}
}

void AMatchPlayerState::CleanupAbilitySystemLeftOver()
{
	for (const FActiveGameplayEffectHandle& ActiveEffectHandle : AbilitySystemComponent->GetActiveGameplayEffects().GetAllActiveEffectHandles())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectHandle);
	}
	
	if (!HasAuthority()) return;
	
	SetCharacterData(nullptr);
	RemoveCurrentTeamAbilitySet();
	ResetTags();
}

void AMatchPlayerState::RemovePawnRelatedTags() const
{
	for (const FGameplayTag& Tag : TagsToRemoveOnNewPawn)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(Tag, 0);

		if (!HasAuthority()) continue;
		
		AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(Tag, 0);
	}
}

void AMatchPlayerState::AddPawnRelatedTags() const
{
	if (!HasAuthority()) return;
	
	for (const FGameplayTag& Tag : TagsToAddOnNewPawn)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(Tag, 1);
		AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(Tag, 1);
	}
}

void AMatchPlayerState::ResetTags() const
{
	RemovePawnRelatedTags();
	AddPawnRelatedTags();
}

void AMatchPlayerState::HandleJoinAttackerTeam()
{
	MatchGameState->OnNewAttacker.Broadcast(this);
}

void AMatchPlayerState::HandleJoinDefenderTeam()
{
	MatchGameState->OnNewDefender.Broadcast(this);
}

void AMatchPlayerState::HandleJoinSpectatorTeam()
{
}

bool AMatchPlayerState::IsDefenderTeam() const
{
	const FToyboxGameplayTags NativeTags = FToyboxGameplayTags::Get();
	if (!HasAuthority())
	{
		return AbilitySystemComponent->HasMatchingGameplayTag(NativeTags.Team_Defender);
	}
	return CurrentTeamTag == NativeTags.Team_Defender;
}

bool AMatchPlayerState::IsAttackerTeam() const
{
	const FToyboxGameplayTags NativeTags = FToyboxGameplayTags::Get();
	if (!HasAuthority())
	{
		return AbilitySystemComponent->HasMatchingGameplayTag(NativeTags.Team_Attacker);
	}

	return CurrentTeamTag == NativeTags.Team_Attacker;
}

bool AMatchPlayerState::IsSpectatorTeam() const
{
	const FToyboxGameplayTags NativeTags = FToyboxGameplayTags::Get();

	if (!IsValid(AbilitySystemComponent))
	{
		return true;
	}
	
	if (!HasAuthority())
	{
		return AbilitySystemComponent->HasMatchingGameplayTag(NativeTags.Team_Spectator);
	}

	return CurrentTeamTag == NativeTags.Team_Spectator;
}

FGameplayTag AMatchPlayerState::GetTeamTag() const
{	
	return CurrentTeamTag;
}

void AMatchPlayerState::GrantCharacterData()
{
	if (IsValid(CharacterData))
	{
		AbilitySystemComponent->SetTagRelationshipMapping(CharacterData->TagRelationshipMapping);
		OnInputConfigSet.ExecuteIfBound(CharacterData->InputConfig);
		GrantAbilitySets(CharacterData->AbilitySets);

		OnCharacterDataInitialized.Broadcast(CharacterData);
	}
}

void AMatchPlayerState::GrantAbilitySets(TArray<TObjectPtr<UToyboxAbilitySet>> AbilitySetsToGrant)
{
	if (!HasAuthority())
	{
		return;
	}
	for (TWeakObjectPtr<UToyboxAbilitySet> AbilitySet : AbilitySetsToGrant)
	{
		if (AbilitySet.IsValid())
		{
			GrantAbilitySet(AbilitySet, AbilitySet_GrantedHandles);
		}
	}
}

void AMatchPlayerState::BeginPlay()
{
	Super::BeginPlay();

	ToyboxPlayerController = Cast<AToyboxPlayerController>(GetOwningController());

	if (!HasAuthority())
	{
		const FToyboxGameplayTags& NativeTags = FToyboxGameplayTags::Get();
		AbilitySystemComponent->RegisterGameplayTagEvent(NativeTags.Team_Attacker).AddUObject(this, &ThisClass::OnTeamTagChanged);
		AbilitySystemComponent->RegisterGameplayTagEvent(NativeTags.Team_Spectator).AddUObject(this, &ThisClass::OnTeamTagChanged);
		AbilitySystemComponent->RegisterGameplayTagEvent(NativeTags.Team_Defender).AddUObject(this, &ThisClass::OnTeamTagChanged);
	}
}

void AMatchPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AbilitySet_GrantedHandles.TakeFromAbilitySystem(GetToyboxAbilitySystemComponent());
	
	Super::EndPlay(EndPlayReason);
}

void AMatchPlayerState::UpdateDeathCount()
{
	if (!IsAlive())
	{
		DeathCount++;
		NotifyGameStatePawnDied();
		//notify about death
	}
	else
	{
		//setting to 0 here, because we have no new pawn and our old pawn got not killed, so we are resetting
		DeathCount = 0;
	}
}

void AMatchPlayerState::GrantCharacterDataForBattleState()
{
	if (!MatchGameState.IsValid()) return;
			
	if (MatchGameState->IsGameModeInBattle())
	{
		SetCharacterData(ChampionData->RelatedCharacterData.LoadSynchronous());
		GrantTeamAbilitySet();
	}
	else if (MatchGameState->IsGameModeInPreparation())
	{
		SetCharacterData(PreparationCharacterData);
	}
}

void AMatchPlayerState::OnTeamTagChanged(FGameplayTag ChangedTeamTag, const int32 NewCount)
{
	if (NewCount > 0)
	{
		CurrentTeamTag = ChangedTeamTag;
		OnTeamChanged();
	}
	else
	{
		CurrentTeamTag = FGameplayTag::EmptyTag;
	}
}

void AMatchPlayerState::OnNewPawnSet(APlayerState* PlayerState, APawn* NewPawn, APawn* OldPawn)
{
	if (!HasAuthority())
	{
		ResetTags();
	}
	//Scope to prevent init ability actor info to be inside the trace
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(AMatchPlayerState::OnNewPawnSet)
	
		if (NewPawn == nullptr && OldPawn != nullptr)
		{
			UpdateDeathCount();
			//Pawn got destroyed, clean up character data
			CleanupAbilitySystemLeftOver();
		}
		else
		{
			if (OldPawn != nullptr)
			{
				//clear old values before regranting
				CleanupAbilitySystemLeftOver();
			}
		
			if (HasAuthority())
			{
				//only authority because the current battle state is not yet replicated
				GrantCharacterDataForBattleState();
			}
		}
	}
	
	AbilitySystemComponent->InitAbilityActorInfo(this, NewPawn);
}

void AMatchPlayerState::OnRep_SelectedChampion()
{
	Super::OnRep_SelectedChampion();
	
	GetChampionData_Internal();
}

void AMatchPlayerState::OnRep_CharacterData()
{
	CleanupCharacterData();
	
	GrantCharacterData();
}

void AMatchPlayerState::GetChampionData_Internal()
{
	if (!ensure(IsValid(ChampionDataTable))) return;

	TArray<FChampionData*> Champions;
	ChampionDataTable->GetAllRows<FChampionData>(TEXT("PlayerState find ChampionData"), Champions);
	for (FChampionData* Champion : Champions)
	{
		if (Champion != nullptr && Champion->Champion == SelectedChampion)
		{
			ChampionData = Champion;
			break;
		}
	}
}

void AMatchPlayerState::SetCharacterData(TWeakObjectPtr<UToyboxCharacterData> NewCharacterData)
{
	CharacterData = NewCharacterData.Get();
	OnRep_CharacterData();
}

void AMatchPlayerState::CleanupCharacterData()
{
	AbilitySet_GrantedHandles.TakeFromAbilitySystem(AbilitySystemComponent);
	AbilitySystemComponent->SetTagRelationshipMapping(nullptr);
	OnInputConfigSet.ExecuteIfBound(nullptr);
}

void AMatchPlayerState::GrantAbilitySet(const TWeakObjectPtr<UToyboxAbilitySet> NewSet, FToyboxAbilitySet_GrantedHandles& OutGrantedHandles) const
{
	if (!ensure(NewSet.IsValid())) return;
	NewSet->GiveToAbilitySystem(AbilitySystemComponent, &OutGrantedHandles);
}

void AMatchPlayerState::GrantTeamAbilitySet()
{
	if (IsAttackerTeam())
	{
		GrantAbilitySet(AttackerAbilitySet, TeamAbilitySet_GrantedHandles);
	}
	else if (IsDefenderTeam())
	{
		GrantAbilitySet(DefenderAbilitySet, TeamAbilitySet_GrantedHandles);
	}
}

void AMatchPlayerState::RemoveCurrentTeamAbilitySet()
{
	TeamAbilitySet_GrantedHandles.TakeFromAbilitySystem(AbilitySystemComponent);
}

FChampionData* AMatchPlayerState::GetChampionData()
{
	return ChampionData;
}

TWeakObjectPtr<UStaticMesh> AMatchPlayerState::GetSpawnPointMesh() const
{
	return ChampionData != nullptr ? ChampionData->SpawnPointMesh.LoadSynchronous() : nullptr;
}

UToyboxAbilitySystemComponent* AMatchPlayerState::GetToyboxAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* AMatchPlayerState::GetAbilitySystemComponent() const
{
	return GetToyboxAbilitySystemComponent();
}

TWeakObjectPtr<UToyboxCharacterData> AMatchPlayerState::GetCharacterData() const
{
	return CharacterData;
}

TWeakObjectPtr<UToyboxInputConfig> AMatchPlayerState::GetInputConfig() const
{
	return IsValid(CharacterData) ? CharacterData->InputConfig : nullptr;
}

bool AMatchPlayerState::IsAlive() const
{
	const FToyboxGameplayTags NativeTags = FToyboxGameplayTags::Get();
	return !AbilitySystemComponent->HasMatchingGameplayTag(NativeTags.State_Dead);
}
