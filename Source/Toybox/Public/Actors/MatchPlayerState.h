// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ToyboxPlayerState.h"
#include "HelperObjects/ToyboxAbilitySet.h"
#include "MatchPlayerState.generated.h"

class AMatchGameState;
class AToyboxPlayerController;
struct FToyboxAbilitySet_GrantedHandles;
class UToyboxInputConfig;
class UToyboxAbilityTagRelationshipMapping;
class UToyboxAbilitySet;
class UToyboxCharacterData;
class UToyboxAbilitySystemComponent;
/**
 * 
 */

DECLARE_DELEGATE_OneParam(FOnInputConfigSet, TWeakObjectPtr<UToyboxInputConfig>);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterDataInitialized, TWeakObjectPtr<UToyboxCharacterData>);

UCLASS()
class TOYBOX_API AMatchPlayerState : public AToyboxPlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMatchPlayerState();

	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FOnCharacterDataInitialized OnCharacterDataInitialized;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void UpdateDeathCount();
	void GrantCharacterDataForBattleState();

	void OnTeamTagChanged(FGameplayTag ChangedTeamTag, const int32 NewCount);

	UFUNCTION()
	void OnNewPawnSet(APlayerState* PlayerState, APawn* NewPawn, APawn* OldPawn);
	
	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UToyboxAbilitySystemComponent> AbilitySystemComponent = {nullptr};

	UPROPERTY(EditAnywhere, Category = "Champion")
	EChampion DefaultChampion = EChampion::Phelani;

	//will always be null if it is not granted so we dont make missassumptions about the current state of the AbilitySystem
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_CharacterData);
	TObjectPtr<UToyboxCharacterData> CharacterData = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Match|Preperation")
	TObjectPtr<UToyboxCharacterData> PreparationCharacterData = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Match|Battle")
	TObjectPtr<UToyboxAbilitySet> AttackerAbilitySet = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Match|Battle")
	TObjectPtr<UToyboxAbilitySet> DefenderAbilitySet = {nullptr};

	UPROPERTY(EditDefaultsOnly, Category = "Champion")
	TObjectPtr<UDataTable> ChampionDataTable = {nullptr};

	FChampionData* ChampionData = {nullptr};
	
	FToyboxAbilitySet_GrantedHandles AbilitySet_GrantedHandles;
	FToyboxAbilitySet_GrantedHandles TeamAbilitySet_GrantedHandles;

	TWeakObjectPtr<AToyboxPlayerController> ToyboxPlayerController = {nullptr};

	virtual void OnRep_SelectedChampion() override;

	UFUNCTION()
	virtual void OnRep_CharacterData();
	
	void GetChampionData_Internal();
	void SetCharacterData(TWeakObjectPtr<UToyboxCharacterData> NewCharacterData);
	void CleanupCharacterData();
	void GrantCharacterData();
	void GrantAbilitySets(TArray<TObjectPtr<UToyboxAbilitySet>> AbilitySetsToGrant);

	void GrantAbilitySet(const TWeakObjectPtr<UToyboxAbilitySet> NewSet, FToyboxAbilitySet_GrantedHandles& OutGrantedHandles) const;
	void GrantTeamAbilitySet();
	void RemoveCurrentTeamAbilitySet();
	
public:
	void SetTeam(const FGameplayTag& NewTeamTag);

	bool IsDefenderTeam() const;

	bool IsAttackerTeam() const;

	bool IsSpectatorTeam() const;

	FGameplayTag GetTeamTag() const;
	
	FOnInputConfigSet OnInputConfigSet;
	
protected:
	void OnTeamChanged();

	void OnGameStateSet(AGameStateBase* NewGameState);

	
	FGameplayTag CurrentTeamTag = FGameplayTag::EmptyTag;

	void NotifyGameStatePawnDied() const;
	
	void CleanupAbilitySystemLeftOver();

	void RemovePawnRelatedTags() const;
	void AddPawnRelatedTags() const;

	void ResetTags() const;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer TagsToRemoveOnNewPawn;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer TagsToAddOnNewPawn;

	int32 DeathCount = 0;
	
private:
	virtual void HandleJoinAttackerTeam();

	virtual void HandleJoinDefenderTeam();

	virtual void HandleJoinSpectatorTeam();

	TWeakObjectPtr<AMatchGameState> MatchGameState = {nullptr};
	
public:
	// Simple Getters
	FChampionData* GetChampionData();
	
	TWeakObjectPtr<UStaticMesh> GetSpawnPointMesh() const;
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	UToyboxAbilitySystemComponent* GetToyboxAbilitySystemComponent() const;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	TWeakObjectPtr<UToyboxCharacterData> GetCharacterData() const;
	TWeakObjectPtr<UToyboxInputConfig> GetInputConfig() const;

	bool IsAlive() const;
};
