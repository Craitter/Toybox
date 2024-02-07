// Fill out your copyright notice in the Description page of Project Settings.


#include "HelperObjects/ToyboxGameplayTags.h"
#include "GameplayTagsManager.h"
#include "Engine/EngineTypes.h"

FToyboxGameplayTags FToyboxGameplayTags::GameplayTags;

void FToyboxGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& GameplayTagsManager = UGameplayTagsManager::Get();

	GameplayTags.AddAllTags(GameplayTagsManager);

	GameplayTagsManager.DoneAddingNativeTags();
}

void FToyboxGameplayTags::AddAllTags(UGameplayTagsManager& Manager)
{
	AddTag(InputTag_Move, "InputTag.Move", "Move Input.");
	AddTag(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	AddTag(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");

	AddTag(InputTag_Ability_Sprint, "InputTag.Ability.Sprint", "Start and stop sprinting");
	AddTag(InputTag_Ability_Jump, "InputTag.Ability.Jump", "Init a Jump");
	AddTag(InputTag_Ability_Dash, "InputTag.Ability.Dash", "Init a Dash");
	AddTag(InputTag_Ability_AimDownSight, "InputTag.Ability.AimDownSight", "Aim down sight");
	AddTag(InputTag_Ability_FlipOffset, "InputTag.Ability.FlipOffset", "Flip camera offset side input");
	AddTag(InputTag_Ability_SwapWeapon, "InputTag.Ability.SwapWeapon", "Input to Init Swap Weapon");
	AddTag(InputTag_Ability_DrawMelee, "InputTag.Ability.DrawMelee", "Input Draw Melee");
	AddTag(InputTag_Ability_DrawRanged, "InputTag.Ability.DrawRanged", "Input Draw RangedWeapon");
	AddTag(InputTag_Ability_AttackMelee, "InputTag.Ability.AttackMelee", "Input Attack With Melee Weapon");
	AddTag(InputTag_Ability_AttackRanged, "InputTag.Ability.AttackRanged", "Input Attack With Ranged Weapon");
	AddTag(InputTag_Ability_Reload, "InputTag.Ability.Reload", "Input Reload Held Weapon");
	AddTag(InputTag_Ability_Special, "InputTag.Ability.Special", "Trigger for special Ability");
	AddTag(InputTag_Ability_Throwable, "InputTag.Ability.Throwable", "Trigger for Throwable Ability");
	
	AddTag(MovementState_Airborne, "MovementState.Airborne", "Tag Added when Airborne");
	
	AddTag(AbilityTag_Movement_Dash, "AbilityTag.Movement.Dash", "Dash Ability Tag");
	AddTag(AbilityTag_Movement_Jump, "AbilityTag.Movement.Jump", "Jump Ability Tag");
	AddTag(AbilityTag_Movement_Sprint, "AbilityTag.Movement.Sprint", "Sprint Ability Tag");
	AddTag(AbilityTag_Weapon_AimDownSight, "AbilityTag.Weapon.AimDownSight", "Aim Down Sight Ability Tag");
	AddTag(AbilityTag_Camera_FlipOffset, "AbilityTag.Camera.FlipOffset", "Flip camera offset Ability Tag");
	AddTag(AbilityTag_Weapon_Recoil, "AbilityTag.Weapon.Recoil", "Weapon Recoil Ability Tag");
	AddTag(AbilityTag_Combat_SwapWeapon, "AbilityTag.Combat.SwapWeapon", "Swap the Current Weapon");
	AddTag(AbilityTag_Combat_DrawMelee, "AbilityTag.Combat.DrawMelee", "Draw Melee weapon Ability Tag");
	AddTag(AbilityTag_Combat_DrawRanged, "AbilityTag.Combat.DrawRanged", "Draw Ranged Ability Tag");
	AddTag(AbilityTag_Combat_AttackMelee, "AbilityTag.Combat.AttackMelee", "Melee Attack Ability Tag");
	AddTag(AbilityTag_Combat_AttackRanged, "AbilityTag.Combat.AttackRanged", "Ranged Attack Ability Tag");
	AddTag(AbilityTag_Combat_Reload, "AbilityTag.Combat.Reload", "Reload Ability Tag");
	AddTag(AbilityTag_Combat_Special, "AbilityTag.Combat.Special", "Special Ability Tag");
	AddTag(AbilityTag_Combat_Throwable, "AbilityTag.Combat.Throwable", "Throwable Ability Tag");


	AddTag(Weapon, "Weapon", "Tag to expose the current weapon ");
	AddTag(Weapon_Melee, "Weapon.Melee", "Tag to expose that the current weapon is Melee");
	AddTag(Weapon_Ranged, "Weapon.Ranged", "Tag to expose that the current weapon is Ranged");
	AddTag(Weapon_Unarmed, "Weapon.Unarmed", "Tag to show that there is no weapon equipped currently");

	AddTag(CameraModifier_DashCameraMod, "CameraModifier.Dash", "The Dash Modifier identifies with this tag");

	AddTag(MetaTag_Damage, "MetaTag.Damage", "Damage Meta tag (How much damage should this effect do)");
	AddTag(MetaTag_BulletSpread, "MetaTag.BulletSpread", "BulletSpread Meta tag (How much BulletSpread should this effect add)");
	AddTag(MetaTag_Heal, "MetaTag.Heal", "Heal Meta tag (How much Heal should this effect do)");
	AddTag(MetaTag_Shield, "MetaTag.Shield", "Shield Meta tag (How much Shield should this effect do)");
	AddTag(MetaTag_Duration, "MetaTag.Duration", "Duration Meta tag (How long an effect should last)");
	AddTag(MetaTag_Buff_DamageAbsolute, "MetaTag.Buff.DamageAbsolute", "DamageBuffAbsolute Meta tag (How much more damage do we deal)");
	

	AddTag(State_Dead, "State.Dead", "Tag used to tell if a character is dead");
	AddTag(State_AimDownSights, "State.ADS", "Tag used to tell if a character is Aiming Down Sights");
	AddTag(State_Slowed, "State.Slowed", "Tag to apply and remove slow effect on a character");
	
	AddTag(Team, "Team", "Base Tag for Teams");
	AddTag(Team_Attacker, "Team.Attacker", "Tag to show which team we are on");
	AddTag(Team_Defender, "Team.Defender", "Tag to show which team we are on");
	AddTag(Team_Spectator, "Team.Spectator", "Tag to show which team we are on");
	
	AddTag(Pawn, "Pawn", "Tag to show that the Avatar Actor is a Pawn");

	AddTag(GameplayCue_Character_DamageTaken, "GameplayCue.Character.DamageTaken", "Cue Tag to be able to call the DamageTaken Cue");
}

void FToyboxGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName), FString(TEXT("(Native) ")) + FString(TagComment));
}