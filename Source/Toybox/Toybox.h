// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//this is only true as long as we stay away from splitscreen
#define LOCAL_USER_INDEX					0

#define DESIRED_PLAYER_COUNT				4

#define DECAL_FADE_SCREEN_SIZE				0.0015;

#define PLAYER_START_DEFENDER			TEXT("Defender")

#define PLAYER_START_ATTACKER			TEXT("Attacker")

#define BULLET_OBJECT_CHANNEL			ECollisionChannel::ECC_GameTraceChannel1

#define BULLET_COLLISION_PROFILE		FName("Bullet")

#define PROFILE_OVERLAP_ALL				FName("OverlapAll")

#define PROFILE_NO_COLLISION			FName("NoCollision")

//Custom Logging
//https://blog.jamie.holdings/2020/04/21/unreal-engine-4-custom-log-categories/
DECLARE_LOG_CATEGORY_EXTERN(LogToyboxOnlineService, Log, All);

DECLARE_LOG_CATEGORY_EXTERN(LogToyboxCamera, Log, All);

DECLARE_LOG_CATEGORY_EXTERN(LogToyboxAbilitySystem, Log, All);

TOYBOX_API FString GetClientServerContextString(UObject* ContextObject = nullptr);

TOYBOX_API FString GetBoolAsString(bool Value);

//Level Streaming

TOYBOX_API int32 GetUniqueLevelStreamingId();

TOYBOX_API FString MakeLevelStreamingName(const int32 Index);