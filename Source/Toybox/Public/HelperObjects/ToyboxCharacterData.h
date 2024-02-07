// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ToyboxCharacterData.generated.h"

class UToyboxInputConfig;
class UToyboxAbilityTagRelationshipMapping;
class UToyboxAbilitySet;
/**
 * 
 */
UCLASS()
class TOYBOX_API UToyboxCharacterData : public UDataAsset
{
	GENERATED_BODY()

public:
	UToyboxCharacterData(const FObjectInitializer& ObjectInitializer);
	
	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toybox|Abilities")
	TArray<TObjectPtr<UToyboxAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toybox|Abilities")
	TObjectPtr<UToyboxAbilityTagRelationshipMapping> TagRelationshipMapping = {nullptr};

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Toybox|Input")
	TObjectPtr<UToyboxInputConfig> InputConfig = {nullptr};
};
