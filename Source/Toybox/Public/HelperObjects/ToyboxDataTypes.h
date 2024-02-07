// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"

#include "ToyboxDataTypes.generated.h"

/**
* This file is used to store global data types (Structs and Enums)
**/

class AThrowableProjectile;
class UMeleeWeaponData;
class URangedWeaponData;
class ARangedProjectile;
class UToyboxCharacterData;

//When editing the Enum, make sure that you update the First and Last so we dont break our current iterations
UENUM(BlueprintType)
enum class EToyline
{
	None,
	
	EForce				UMETA(DisplayName = "E-Force"),

	Stormasaurs			UMETA(DisplayName = "Stormasaurs"),

	KnaveDanger			UMETA(DisplayName = "Knave Danger"),

	GalacticRebels		UMETA(DisplayName = "Galactic Rebels"),

	Max				UMETA(Hidden),
};

//When editing the Enum, make sure that you update the First and Last so we dont break our current iterations
ENUM_RANGE_BY_FIRST_AND_LAST(EToyline, EToyline::EForce, EToyline::GalacticRebels);

UENUM(BlueprintType)
enum class EFaction
{
	None,
	
	Hero				UMETA(DisplayName = "Hero"),

	Villain				UMETA(DisplayName = "Villain"),

	Max					UMETA(Hidden),
};


UENUM(BlueprintType)
enum class EChampion
{
	None,
	
	Charlie_Foxtrot				UMETA(DisplayName = "Charlie Foxtrot"),

	Princess_Rexa				UMETA(DisplayName = "Princess Rexa"),

	Phelani						UMETA(DisplayName = "Phelani"),

	Warlord_Kane				UMETA(DisplayName = "Warlord Kane"),

	Max							UMETA(Hidden),
};

UENUM(BlueprintType)
enum class ERangedWeaponOperatingType
{
	None,
	
	Single						UMETA(DisplayName = "Single Fire"),

	Continuous					UMETA(DisplayName = "Continuous Fire (Automatic)"),

	Burst						UMETA(DisplayName = "Burst Fire"),
	
	Max							UMETA(Hidden),
};

USTRUCT(BlueprintType)
struct FMeleeAttack
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	float Damage = 0.0f;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> Animation = {nullptr};
};

USTRUCT(BlueprintType)
struct FAbilityUi
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag CooldownTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> ActiveIcon = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> InactiveIcon = {nullptr};
};

USTRUCT()
struct FChampionData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	EToyline Toyline = EToyline::None;

	UPROPERTY(EditDefaultsOnly)
	EFaction Faction = EFaction::None;

	UPROPERTY(EditDefaultsOnly)
	EChampion Champion = EChampion::None;

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "Weapon"))
	FGameplayTag PreviewEquippedWeapon = FGameplayTag::EmptyTag;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UToyboxCharacterData> RelatedCharacterData = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> RelatedMesh = {nullptr};
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimationAsset> RelatedIdleAnimation = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<URangedWeaponData> RangedWeaponData;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMeleeWeaponData> MeleeWeaponData;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AThrowableProjectile> ThrowableProjectile = {nullptr};
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UStaticMesh> SpawnPointMesh = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	float DeadRagdollLifeSpan = 5.0f;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> SpecialAbilityMontage = {nullptr};

	UPROPERTY(EditDefaultsOnly)
	int32 OutlineStencilValue = 2;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	FAbilityUi ThrowableUiIcons;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	FAbilityUi SpecialUiIcons;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TObjectPtr<UTexture2D> CharacterIcon = {nullptr};
	
	bool operator==(const FChampionData& Data) const
	{
		return Champion == Data.Champion;
	}

	bool operator!=(const FChampionData& Data) const
	{
		return Champion != Data.Champion;
	}
};

USTRUCT()
struct FMeleeCollisionData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	FVector RelativeOffset = FVector::Zero();

	UPROPERTY(EditDefaultsOnly)
	float Radius = 32.0f;
};