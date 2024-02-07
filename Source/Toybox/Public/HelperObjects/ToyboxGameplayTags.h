// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UGameplayTagsManager;

/**
 * ToyboxGameplayTags
 *
 * Singleton containing native gameplay tags
 */

struct TOYBOX_API FToyboxGameplayTags
{
public:
	static const FToyboxGameplayTags& Get() { return GameplayTags; }

	static void InitializeNativeTags();

	//Input tags
	FGameplayTag InputTag_Move;
	FGameplayTag InputTag_Look_Mouse;
	FGameplayTag InputTag_Look_Stick;

	FGameplayTag InputTag_Ability_Sprint;
	FGameplayTag InputTag_Ability_Jump;
	FGameplayTag InputTag_Ability_Dash;
	FGameplayTag InputTag_Ability_AimDownSight;
	FGameplayTag InputTag_Ability_FlipOffset;
	FGameplayTag InputTag_Ability_SwapWeapon;
	FGameplayTag InputTag_Ability_DrawMelee;
	FGameplayTag InputTag_Ability_DrawRanged;
	FGameplayTag InputTag_Ability_AttackMelee;
	FGameplayTag InputTag_Ability_AttackRanged;
	FGameplayTag InputTag_Ability_Reload;
	FGameplayTag InputTag_Ability_Special;
	FGameplayTag InputTag_Ability_Throwable;
	
	FGameplayTag MovementState_Airborne;
	
	FGameplayTag AbilityTag_Movement_Dash;
	FGameplayTag AbilityTag_Movement_Jump;
	FGameplayTag AbilityTag_Movement_Sprint;
	FGameplayTag AbilityTag_Camera_FlipOffset;
	FGameplayTag AbilityTag_Weapon_AimDownSight;
	FGameplayTag AbilityTag_Weapon_Recoil;
	FGameplayTag AbilityTag_Combat_SwapWeapon;
	FGameplayTag AbilityTag_Combat_DrawMelee;
	FGameplayTag AbilityTag_Combat_DrawRanged;
	FGameplayTag AbilityTag_Combat_AttackMelee;
	FGameplayTag AbilityTag_Combat_AttackRanged;
	FGameplayTag AbilityTag_Combat_Reload;
	FGameplayTag AbilityTag_Combat_Special;
	FGameplayTag AbilityTag_Combat_Throwable;
	

	FGameplayTag Weapon;
	FGameplayTag Weapon_Melee;
	FGameplayTag Weapon_Ranged;
	FGameplayTag Weapon_Unarmed;

	FGameplayTag CameraModifier_DashCameraMod;

	FGameplayTag MetaTag_Damage;
	FGameplayTag MetaTag_BulletSpread;
	FGameplayTag MetaTag_Heal;
	FGameplayTag MetaTag_Shield;
	FGameplayTag MetaTag_Duration;
	FGameplayTag MetaTag_Buff_DamageAbsolute;
	
	FGameplayTag State_Dead;
	FGameplayTag State_Slowed;
	FGameplayTag State_AimDownSights;

	FGameplayTag Team;
	FGameplayTag Team_Attacker;
	FGameplayTag Team_Defender;
	FGameplayTag Team_Spectator;

	FGameplayTag Pawn;

	FGameplayTag GameplayCue_Character_DamageTaken;

protected:
	//Registers all of the tags with the GameplayTags Manager
	void AddAllTags(UGameplayTagsManager& Manager);

	//Helper function used to create a single tag with the GameplayTags Manager
	static void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);

private:
	static FToyboxGameplayTags GameplayTags;
};


